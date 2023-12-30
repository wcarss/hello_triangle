default: osx

osx: src/ include/
	cd src; make osx;

clean:
	rm -rf hello_triangle
	rm -rf *.o
	cd src; make clean
