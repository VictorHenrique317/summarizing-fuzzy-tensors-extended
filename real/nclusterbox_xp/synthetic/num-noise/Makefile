NAME = num-noise
CXX = clang++ -O3 -flto -Wall -Wextra -Weffc++ -std=c++11 -pedantic
# CXX = g++ -O3 -flto -Wall -Wextra -Weffc++ -std=c++11 -pedantic
SRC = src
OBJ = $(SRC)/num-noise.o
DATA = COPYING INSTALL Makefile README

.PHONY: install clean dist-gzip dist-bzip2 dist-xz dist
.SILENT: $(NAME) install clean dist-gzip dist-bzip2 dist-xz dist

%.o: %.cpp
	$(CXX) -c -o $@ $<

$(NAME): $(OBJ)
	$(CXX) -o $@ $^
	echo "$(NAME) built!"

install: $(NAME)
	mv $(NAME) /usr/bin
	echo "$(NAME) installed!"

clean:
	rm -f $(SRC)/*.o *~ $(SRC)/*~

dist-gzip: clean
	tar --format=posix --transform 's,^,$(NAME)/,' -czf $(NAME).tar.gz $(SRC) $(DATA)

dist-bzip2: clean
	tar --format=posix --transform 's,^,$(NAME)/,' -cjf $(NAME).tar.bz2 $(SRC) $(DATA)

dist-xz: clean
	tar --format=posix --transform 's,^,$(NAME)/,' -cJf $(NAME).tar.bz2 $(SRC) $(DATA)

dist: dist-bzip2
