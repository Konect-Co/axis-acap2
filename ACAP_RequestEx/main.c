/*
https://github.com/yhirose/cpp-httplib
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum RequestType {GET, POST};

struct resultType {
    /*
    fields
    */
   int howMany;
};

char* result2JSON (struct resultType a) {
    char result[200] = "{";
    strcat(result, "\'howMany\' : ");
    //strcat(result, itoa(a.howMany));
    strcat(result, "}");
    return result;
}

int main() {
    enum RequestType type;
    type = POST;
    char request[] = "curl -X POST -d '{\"hello\":\"yho\"}' http://3.137.221.232/sendData";

    printf("Sending request:\n");
    printf("\t%s\n", request);
    system(request);
    printf("Response: \n");
}