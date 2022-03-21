CC = gcc
GCOV = -fprofile-arcs -ftest-coverage
TARGET = SmartCalc
GTK4_PASS = `pkg-config --cflags --libs gtk4-macos` -I/.brew/include -L/.brew/Cellar/gtk4/4.4.1/lib

.PHONY: all clean install uninstall check dvi dist

all: $(TARGET) install clean

$(TARGET): calculate.c gui.c credit_work.c
	$(CC) gui.c calculate.c credit_work.c $(GTK4_PASS) -o $(TARGET)

calculate.o: calculate.c
	gcc -c -o calculate.o calculate.c

credit_work.o: credit_work.c
	gcc -c -o credit_work.o credit_work.c

install: GUI
	mkdir -p ../SmartCalc
	mkdir -p ~/models
	mkdir -p ~/XML
	mkdir -p ~/CSS
	install ./SmartCalc ../SmartCalc
	install ./XML/calculate.xml ~/gui
	install ./XML/credit.xml ~/gui
	install ./XML/menu.xml ~/gui
	install ./XML/plot.xml ~/gui
	install ./CSS/stule.css ~/gui

uninstall:
	rm -rf ../SmartCalc
	rm -rf ~/XML
	rm -rf ~/CSS

dvi:
	open s21_documentation.html

dist: $(TARGET) install clean
	tar -czf SmartCalc.tgz ./*

test: calculate.o credit_work.o
	gcc -c test.c -o test.o
	gcc test.o calculate.o credit_work.o -pthread -lm -o test1.o -lcheck
	./test1.o

gcov_report: calculate.o credit_work.o
	@gcc $(GCOV) test.c -pthread -lcheck -lm calculate.c credit_work.c -o Test
	@./Test
	@lcov -t "Test" -o Test.info -c -d .
	@genhtml -o report Test.info 
	open ./report/index-sort-f.html

check:
	cppcheck *.h *.c
	cp ../materials/linters/CPPLINT.cfg CPPLINT.cfg
	python3 ../materials/linters/cpplint.py --extension=c *.c *.h
	CK_FORK=no leaks --atExit -- ./test1.o

clean:
	rm -rf ./*.o ./*.a ./a.out ./*.gcno ./*.gcda ./report ./*.info ./Test ./CPPLINT.cfg $(TARGET)
