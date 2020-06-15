//
// Created by Joshua Heinemann on 13.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

extern "C"{
       	int printf(const char* message);
}
int __attribute__((optnone)) main(int argc, char** argv){ //Disable O3 optimization specified in WasiToolchain.cmake
	printf("Hello World.\n");
}