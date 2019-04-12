#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "PropertyNode.h"

#ifdef VALUE_USE_BITBUFFERS
    #include "bitbuffers-cpp/BufferReader.h"
    #include "bitbuffers-cpp/VectorBuffer.h"
#endif

using std::cout;
using std::endl;


int main(int argc , char *argv[]) {

    PropertyNode pn;

    pn.propPrintLocal();
    cout << endl;

    pn.propSet("test", 45);

    VectorBuffer buf;
    buf.appendFromString("0x5411AE");
    cout << buf.toHexString() << endl;
    cout << endl;

    pn.propSet("test2", (BufferReader&)buf);

    pn.propPrintLocal();
    cout << endl;

	return EXIT_SUCCESS;
}



