all: template

%: %.cpp
	g++ -g $< -o $@ -Wall -Wextra

clean: template
	rm -f $<
