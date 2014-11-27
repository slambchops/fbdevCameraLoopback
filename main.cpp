#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "camera.h"
#include "display.h"

int main(int argc, char *argv[])
{
	Display disp1("/dev/fb0", 800, 600);

	Camera cam1("/dev/video0", disp1.getW(), disp1.getH());

        disp1.init();
        cam1.init();
	
	int i = 0;
	while (i < 100) {
		disp1.displayFrame(cam1.getFrame(), cam1.getFrameSize());
		i++;
	}

	disp1.deinit();
	cam1.deinit();

	return 0;
}
