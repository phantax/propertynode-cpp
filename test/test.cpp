#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "PropertyNode.h"

using std::cout;
using std::endl;


int main(int argc , char *argv[]) {

    PropertyNode pn;

    pn.propSet<int>("test", 5);

    pn.propSet<std::string>("#test", "bla bla foo bar xxxxx -->");

    pn.propPrintLocal();

	return EXIT_SUCCESS;
}



