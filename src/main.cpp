#include <bits/stdc++.h>
#include "../include/ScreenRecorder.h"

using namespace std;

/* driver function to run the application */
int main()
{
	ScreenRecorder screen_record;

	screen_record.openCamera();
	screen_record.init_outputfile();
	screen_record.CaptureVideoFrames();

	cout<<"\nprogram executed successfully\n";

return 0;
}
