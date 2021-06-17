flags=-g0 -O0 -pthread

library:
	@mipsel-linux-uclibc-g++ -c gophergdk.cpp $(flags) -o gophergdk.o
	@mipsel-linux-uclibc-ar -r gophergdk.a gophergdk.o
	@rm *.o
demo:
	@mipsel-linux-uclibc-g++ demo.cpp gophergdk.a $(flags) -o demo_gopher2.dge
tileset:
	@mipsel-linux-uclibc-g++ tileset.cpp gophergdk.a $(flags) -o tileset_gopher2.dge
battery:
	@mipsel-linux-uclibc-g++ battery_level.cpp gophergdk.a $(flags) -o battery_gopher2.dge
full:
	@make library
	@make demo
	@make tileset
	@make battery