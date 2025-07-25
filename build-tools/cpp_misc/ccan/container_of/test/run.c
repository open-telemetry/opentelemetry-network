#include <ccan/container_of/container_of.h>
#include <ccan/tap/tap.h>

struct foo {
	int a;
	char b;
};

int main(int argc, char *argv[])
{
	struct foo foo = { .a = 1, .b = 2 };
	int *intp = &foo.a;
	char *charp = &foo.b;

	plan_tests(8);
	ok1(container_of(intp, struct foo, a) == &foo);
	ok1(container_of(charp, struct foo, b) == &foo);
	ok1(container_of_var(intp, &foo, a) == &foo);
	ok1(container_of_var(charp, &foo, b) == &foo);

	ok1(container_off(struct foo, a) == 0);
	ok1(container_off(struct foo, b) == offsetof(struct foo, b));
	ok1(container_off_var(&foo, a) == 0);
	ok1(container_off_var(&foo, b) == offsetof(struct foo, b));
	return exit_status();
}
