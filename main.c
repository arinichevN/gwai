#include "app/main.h"

int main() {
	app_begin();
	while (1) {
		app_control();
	}
	putsde("unexpected while break\n");
	return (EXIT_FAILURE);
}
