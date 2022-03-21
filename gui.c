#include <gtk/gtk.h>

#include "smart_calc_v1.h"

static int valueOperation = UNDEFIND;
static int preValueOperation = UNDEFIND;
static int countOpenBracket = 0;
static int countCloseBracket = 0;
static int scaleUp = FALSE;
static int scaleDown = FALSE;
static gdouble scaleX = 50;
static gdouble scaleY = 50;
Stack *stack;
GtkBuilder *buildCredit;
GtkBuilder *buildCalculate;

/* метод открытия калькулятора */
static void calc_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWidget *windowCalculator = GTK_WIDGET(gtk_builder_get_object(buildCalculate, "win"));
    GtkWidget *windowCredit = GTK_WIDGET(gtk_builder_get_object(buildCredit, "windowCredit"));
    gtk_widget_show(windowCalculator);
    gtk_widget_hide(windowCredit);
}

/* метод открытия кредитного калькулятора */
static void credit_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWidget *windowCalculator = GTK_WIDGET(gtk_builder_get_object(buildCalculate, "win"));
    GtkWidget *windowCredit = GTK_WIDGET(gtk_builder_get_object(buildCredit, "windowCredit"));
    gtk_widget_show(windowCredit);
    gtk_widget_hide(windowCalculator);
}

/* закрытия калькулятора */
static void exit_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GApplication *app = (GApplication *)user_data;
    g_application_quit(app);
}

/* массив связывающий xml с методами */
const GActionEntry app_entries[] = {{"calculator", calc_activated, NULL, NULL, NULL},
                                    {"credit", credit_activated, NULL, NULL, NULL},
                                    {"exit", exit_activated, NULL, NULL, NULL}};

/*обработка значений кнопки result в кредитном калькуляторе*/
void print_result_credit(GtkWidget *text, gpointer *data) {
    CreditSting creditString = {NULL, NULL, NULL, NULL, NULL};
    GtkWidget *textSum = GTK_WIDGET(gtk_builder_get_object(buildCredit, "AmountOfCreditText"));
    GtkWidget *textTerm = GTK_WIDGET(gtk_builder_get_object(buildCredit, "CredittermText"));
    GtkWidget *textPercent = GTK_WIDGET(gtk_builder_get_object(buildCredit, "InterestRateText"));
    GtkWidget *buttonAnnuity = GTK_WIDGET(gtk_builder_get_object(buildCredit, "AnnuityCreditCheckButton"));
    GtkWidget *boxMonthOrYear = GTK_WIDGET(gtk_builder_get_object(buildCredit, "creditComboBox"));

    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(buttonAnnuity)))
        creditString.typeCredit = "annuity";
    else
        creditString.typeCredit = "differentiated";
    creditString.sumCredit = (char *)gtk_entry_buffer_get_text(gtk_text_get_buffer(GTK_TEXT(textSum)));
    creditString.termCredit = (char *)gtk_entry_buffer_get_text(gtk_text_get_buffer(GTK_TEXT(textTerm)));
    creditString.percentCredit =
        (char *)gtk_entry_buffer_get_text(gtk_text_get_buffer(GTK_TEXT(textPercent)));
    creditString.monthOrYear =
        (char *)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(boxMonthOrYear));

    Credit creditRes = credit_work(&creditString);

    GtkWidget *monthlyPayment = GTK_WIDGET(gtk_builder_get_object(buildCredit, "MonthlyPaymentAmountText"));
    GtkWidget *overpayment = GTK_WIDGET(gtk_builder_get_object(buildCredit, "OverpaymentOnCreditText"));
    GtkWidget *totalPayment = GTK_WIDGET(gtk_builder_get_object(buildCredit, "TotalPaymentText"));

    char resultStr[30] = "";
    if (creditRes.check == SUCCESS) {
        if (strcmp("annuity", creditString.typeCredit) == 0)
            snprintf(resultStr, sizeof(resultStr), "%.2f", creditRes.monthlyPayment);
        else
            snprintf(resultStr, sizeof(resultStr), "%.2f ... %.2f", creditRes.maxMonthlyPayment,
                     creditRes.minMonthlyPayment);
    } else {
        snprintf(resultStr, sizeof(resultStr), "%s", "Error");
    }
    gtk_entry_buffer_set_text(gtk_text_get_buffer(GTK_TEXT(monthlyPayment)), resultStr, strlen(resultStr));
    if (creditRes.check == SUCCESS)
        snprintf(resultStr, sizeof(resultStr), "%.2f", creditRes.overpayment);
    else
        snprintf(resultStr, sizeof(resultStr), "%s", "Error");
    gtk_entry_buffer_set_text(gtk_text_get_buffer(GTK_TEXT(overpayment)), resultStr, strlen(resultStr));
    if (creditRes.check == SUCCESS)
        snprintf(resultStr, sizeof(resultStr), "%.2f", creditRes.interestCharges);
    else
        snprintf(resultStr, sizeof(resultStr), "%s", "Error");
    gtk_entry_buffer_set_text(gtk_text_get_buffer(GTK_TEXT(totalPayment)), resultStr, strlen(resultStr));
}

/* определяем какой рисовать график */
static void draw_plot(gdouble clipX1, gdouble clipX2, cairo_t *cr) {
    for (gdouble x = clipX1; x < clipX2; x += 0.005) {
        double y = calculate_to_PN(&stack, x);
        double resultAtan = fabs(fabs(atan(y)) - M_PI_2);
        if (resultAtan <= 0.005 || isnan(y))
            cairo_new_sub_path(cr);
        else
            cairo_line_to(cr, x, y);
    }
}

/* рисование сетки */
static void draw_grid(int width, int height, cairo_t *cr, gdouble dx) {
    cairo_set_source_rgba(cr, 0.99, 0.98, 1, 0.2);
    cairo_set_line_width(cr, dx);
    for (double i = 0; i < width; i += 1) {
        cairo_move_to(cr, -width / 2, i);
        cairo_line_to(cr, width / 2, i);
    }
    for (double i = 0; fabs(i) < width; i -= 1) {
        cairo_move_to(cr, -width / 2, i);
        cairo_line_to(cr, width / 2, i);
    }
    for (double i = 0; i < height; i += 1) {
        cairo_move_to(cr, i, -height / 2);
        cairo_line_to(cr, i, height / 2);
    }
    for (double i = 0; fabs(i) < height; i -= 1) {
        cairo_move_to(cr, i, -height / 2);
        cairo_line_to(cr, i, height / 2);
    }
    cairo_stroke(cr);
}

/* рисование графика */
static void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    gdouble dx = 2, dy = 2;
    gdouble i, clipX1 = 0.0, clipY1 = 0.0, clipX2 = 0.0, clipY2 = 0.0;
    if (scaleUp == TRUE && scaleX != 100) {
        scaleX += 5;
        scaleY += 5;
    }
    if (scaleDown == TRUE && scaleX != 5) {
        scaleX -= 5;
        scaleY -= 5;
    }
    cairo_set_source_rgba(cr, 0.55, 0.55, 0.91, 1);
    cairo_paint(cr);
    cairo_translate(cr, 800 / 2, 300 / 2);
    /* измениять scale для разных маштабов */
    cairo_scale(cr, scaleX, -scaleY);
    /* определить границы графика */
    cairo_device_to_user_distance(cr, &dx, &dy);
    cairo_clip_extents(cr, &clipX1, &clipY1, &clipX2, &clipY2);
    /* рисую оси X и Y */
    cairo_set_line_width(cr, dx);
    cairo_set_source_rgba(cr, 0.99, 0.98, 1, 0.5);
    cairo_move_to(cr, clipX1, 0.0);
    cairo_line_to(cr, clipX2, 0.0);
    cairo_move_to(cr, 0.0, clipY1);
    cairo_line_to(cr, 0.0, clipY2);
    cairo_stroke(cr);
    draw_grid(width, height, cr, dx);
    /* рисую графики */
    cairo_set_line_width(cr, dx);
    cairo_set_source_rgba(cr, 0.99, 0.98, 1, 1);
    draw_plot(clipX1, clipX2, cr);
    cairo_stroke(cr);
}

void scale_up(GtkWidget *button, gpointer *data) {
    scaleUp = TRUE;
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(button), draw_function, NULL, NULL);
    if (scaleDown == TRUE) scaleDown = FALSE;
}

void scale_down(GtkWidget *button, gpointer *data) {
    scaleDown = TRUE;
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(button), draw_function, NULL, NULL);
    if (scaleUp == TRUE) scaleUp = FALSE;
}

/* главная функция для рисования графиков */
static void window_plot() {
    scaleX = 30;
    scaleY = 30;
    GtkBuilder *build = gtk_builder_new_from_file("./XML/plot.xml");
    GtkWidget *windowPlot = GTK_WIDGET(gtk_builder_get_object(build, "windowPlot"));
    GtkWidget *area = GTK_WIDGET(gtk_builder_get_object(build, "area"));
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), draw_function, NULL, NULL);
    GtkWidget *overlay = GTK_WIDGET(gtk_builder_get_object(build, "overlay"));
    GtkWidget *gridPlot = GTK_WIDGET(gtk_builder_get_object(build, "gridPlot"));
    gtk_overlay_set_child(GTK_OVERLAY(overlay), area);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), gridPlot);
    gtk_widget_show(windowPlot);
}

/* определения вида операций кнопок */
static void determinate_priority(const char *label, int *valueOperation) {
    if (strcmp(label, "atan") == 0 || strcmp(label, "acos") == 0 || strcmp(label, "asin") == 0) {
        *valueOperation = UOPERATION;
        countOpenBracket++;
    } else if (strcmp(label, "cos") == 0 || strcmp(label, "sin") == 0 || strcmp(label, "tan") == 0) {
        *valueOperation = UOPERATION;
        countOpenBracket++;
    } else if (strcmp(label, "√") == 0 || strcmp(label, "ln") == 0 || strcmp(label, "log") == 0) {
        *valueOperation = UOPERATION;
        countOpenBracket++;
    } else if (strcmp(label, "÷") == 0 || strcmp(label, "^") == 0 || strcmp(label, "×") == 0) {
        *valueOperation = OPERATION;
    } else if (strcmp(label, "-") == 0 || strcmp(label, "+") == 0 || strcmp(label, "mod") == 0) {
        *valueOperation = OPERATION;
    } else if (strcmp(label, "π") == 0 || strcmp(label, "e") == 0) {
        *valueOperation = CONSTANT_OPERATION;
    } else if (strcmp(label, "") == 0) {
        *valueOperation = OPEN_BRACKET;
        countOpenBracket++;
    } else if (strcmp(label, "(") == 0) {
        *valueOperation = OPEN_BRACKET;
        countOpenBracket++;
    } else if (strcmp(label, ")") == 0) {
        *valueOperation = CLOSE_BRACKET;
        countCloseBracket++;
    } else if (strcmp(label, "x") == 0) {
        *valueOperation = X;
    } else {
        *valueOperation = NUMBER;
    }
}

/* удалить последнюю операцию */
static void delete_operation(GtkEntryBuffer *buff) {
    const char *givenStr = gtk_entry_buffer_get_text(buff);
    gtk_entry_buffer_delete_text(buff, strlen(givenStr) - 1, 1);
}

/* автоматическое добавление умножения перед операторами */
static void add_automatic_mult(GtkEntryBuffer *buff) {
    if (preValueOperation == NUMBER) {
        if (valueOperation != OPERATION && valueOperation != NUMBER && valueOperation != CLOSE_BRACKET)
            gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "*", 1);
    } else if (preValueOperation == CLOSE_BRACKET || preValueOperation == CONSTANT_OPERATION ||
               preValueOperation == X) {
        if (valueOperation != OPERATION && valueOperation != CLOSE_BRACKET)
            gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "*", 1);
    }
}

/* проверка операций */
static int check_operation(const char *label, int lenLabel, GtkEntryBuffer *buff) {
    static int valueCheck = SUCCESS;
    if (valueCheck == SUCCESS) preValueOperation = valueOperation;
    determinate_priority(label, &valueOperation);
    if (valueOperation == UOPERATION) valueCheck = SUCCESS;
    add_automatic_mult(buff);
    if (preValueOperation == OPERATION && valueOperation == OPERATION)
        delete_operation(buff);
    else if (preValueOperation == UOPERATION && valueOperation == OPERATION)
        valueCheck = FAILURE;
    if (valueOperation == NUMBER || valueOperation == OPEN_BRACKET || valueOperation == CONSTANT_OPERATION)
        valueCheck = SUCCESS;
    return valueCheck;
}

/* изменение вывода кнопок на экран */
static void write_text_widget_text(const char *label, int lenLabel, GtkEntryBuffer *buff) {
    if (strcmp(label, "atan") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "atan(", 5);
    else if (strcmp(label, "acos") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "acos(", 5);
    else if (strcmp(label, "asin") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "asin(", 5);
    else if (strcmp(label, "sin") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "sin(", 4);
    else if (strcmp(label, "cos") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "cos(", 4);
    else if (strcmp(label, "tan") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "tan(", 4);
    else if (strcmp(label, "mod") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "%", 1);
    else if (strcmp(label, "ln") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "ln(", 3);
    else if (strcmp(label, "log") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "log(", 4);
    else if (strcmp(label, "×") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "*", 1);
    else if (strcmp(label, "÷") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "/", 1);
    else if (strcmp(label, "√") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "sqrt(", 5);
    else if (strcmp(label, "π") == 0)
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), "pi", 2);
    else
        gtk_entry_buffer_insert_text(buff, gtk_entry_buffer_get_length(buff), label, lenLabel);
}

/* фукция вывода текста на виджет text */
void print_text(GtkWidget *text, gpointer *data) {
    GtkEntryBuffer *buff = gtk_text_get_buffer(GTK_TEXT(text));
    const char *label = gtk_button_get_label(GTK_BUTTON(data));
    int lenLabel = strlen(label);
    int check = check_operation(label, lenLabel, buff);
    if (check == SUCCESS) write_text_widget_text(label, lenLabel, buff);
}

/* фукция вывода резульатат на виджет text | button '=' | */
void print_result(GtkWidget *text, gpointer *data) {
    GtkEntryBuffer *buff = gtk_text_get_buffer(GTK_TEXT(text));
    const char *givenStr = gtk_entry_buffer_get_text(buff);
    int divBracket = countOpenBracket - countCloseBracket;
    if (divBracket == 0) {
        stack = convert_to_polish_notashion(givenStr);
        if (check_value_x_number(stack) == SUCCESS) {
            window_plot();
        } else {
            double result = calculate_to_PN(&stack, UNDEFIND);
            char resultStr[20] = "";
            if (result < 1e-16) result == 0;
            snprintf(resultStr, sizeof(resultStr), "%g", result);
            gtk_entry_buffer_set_text(buff, resultStr, strlen(resultStr));
        }
        countOpenBracket = 0;
        countCloseBracket = 0;
    }
}

/* фукция вывода резульатат на виджет text | button 'AC' | */
void clean_result(GtkWidget *text, gpointer *data) {
    GtkEntryBuffer *buff = gtk_text_get_buffer(GTK_TEXT(text));
    gtk_entry_buffer_set_text(buff, "", 0);
    valueOperation = UNDEFIND;
    preValueOperation = UNDEFIND;
    countOpenBracket = 0;
    countCloseBracket = 0;
}

/* подключение css к gtk */
static void load_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    const gchar *cssStyleFile = "./CSS/style.css";
    GFile *cssFp = g_file_new_for_path(cssStyleFile);
    gtk_css_provider_load_from_file(provider, cssFp);
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER);
}

/* подргузка всех виджететов */
static void activate(GtkApplication *app, gpointer user_data) {
    buildCalculate = gtk_builder_new_from_file("./XML/calculate.xml");
    GtkWidget *win = gtk_application_window_new(GTK_APPLICATION(app));
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(buildCalculate, "win"));
    gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
    buildCredit = gtk_builder_new_from_file("./XML/credit.xml");
    GtkWidget *windowCredit = GTK_WIDGET(gtk_builder_get_object(buildCredit, "windowCredit"));
    gtk_window_set_application(GTK_WINDOW(windowCredit), GTK_APPLICATION(app));
    GtkBuilder *builder = gtk_builder_new_from_file("./XML/menu.xml");
    GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menubar"));
    gtk_application_set_menubar(GTK_APPLICATION(app), menu);
    g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
    load_css();
    gtk_widget_show(window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
