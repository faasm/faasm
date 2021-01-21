extern "C"
{
#include <faasm/emscripten.h>
}

#include <string>

int main()
{
    char buf[25];

    const char* foo = "foobar";
    int n = 123;
    siprintf(buf, "Hi %s %i\n", foo, n);

    std::string expected = "Hi foobar 123\n";
    std::string actual(buf, expected.size());

    if (expected != actual) {
        printf("siprintf not as expected: %s != %s\n",
               expected.c_str(),
               actual.c_str());
        return 1;
    }

    return 0;
}
