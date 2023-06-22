NAME = nclusterbox
# CXX = clang++ -O3 -flto -Wall -Wextra -Weffc++ -pedantic -Wno-unused-parameter
# CXX = g++ -O3 -flto -Wall -Wextra -Weffc++ -pedantic -Wno-unused-parameter
CXX = g++ -g -Og -Wall -Wextra -Weffc++ -pedantic -Wno-unused-parameter
EXTRA_CXXFLAGS = -lboost_program_options -lpthread
HELP2MAN = help2man -n 'Modify patterns, which hold in a fuzzy tensor, to maximize their explanatory powers and select an ordered subset of the built patterns to summarize this tensor' -N
SRC = src/utilities src/core
DEPS = $(wildcard $(patsubst %,%/*.h,$(SRC))) Parameters.h Makefile
CODE = $(wildcard $(patsubst %,%/*.cpp,$(SRC)))
OBJ = $(patsubst %.cpp,%.o,$(CODE))
ALL = $(DEPS) $(CODE) $(NAME).1.gz COPYING INSTALL README slice-input fiber-input reduced-fiber-input-one-way reduced-fiber-input

.PHONY: man install clean dist-gzip dist-bzip2 dist-xz dist
.SILENT: all $(NAME) man install clean dist-gzip dist-bzip2 dist-xz dist

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $<

all: man

$(NAME): $(OBJ)
	$(CXX) -o $@ $^ $(EXTRA_CXXFLAGS)
	echo "$(NAME) built!"

man: $(NAME)
	$(HELP2MAN) ./$(NAME) | gzip > $(NAME).1.gz

install: man
	mv $(NAME) /usr/bin
	cp $(NAME).1.gz /usr/share/man/man1/
	echo "$(NAME) installed!"

clean:
	rm -f $(patsubst %,%/*.o,$(SRC)) $(patsubst %,%/*~,$(SRC)) *~

dist-gzip: man
	tar --format=posix --transform 's,^,$(NAME)/,' -czf $(NAME).tar.gz $(ALL)

dist-bzip2: man
	tar --format=posix --transform 's,^,$(NAME)/,' -cjf $(NAME).tar.bz2 $(ALL)

dist-xz: man
	tar --format=posix --transform 's,^,$(NAME)/,' -cJf $(NAME).tar.xz $(ALL)

dist: dist-bzip2
