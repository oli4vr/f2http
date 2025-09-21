all: main deb
main:
	gcc f2http.c -o f2http -O3
deb:
	/bin/bash ./debpkg.sh
install:
	cp f2http /bin/f2http
uninstall:
	rm -rf /bin/f2http
clean:
	rm -rf f2http *.o *.deb