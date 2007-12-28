#include "lightstone.h"

int lightstone_get_count()
{
	return 0;
}

int lightstone_open(lightstone* dev, unsigned int device_index)
{	
	int i;
	hid_return ret;
	HIDInterfaceMatcher matcher;
	ret = hid_init();

	*dev = hid_new_HIDInterface();
	matcher.vendor_id = LIGHTSTONE_VID;
	matcher.product_id = LIGHTSTONE_PID;
	matcher.matcher_fn = NULL;

	/* open recursively all HID devices found */
	while ( (ret = hid_force_open(*dev, 0, &matcher, 3)) != HID_RET_DEVICE_NOT_FOUND)
	{
		return ret;
	}
	return 0;
}

int lightstone_close(lightstone dev)
{
	//hid_delete_HIDInterface(dev);
	return hid_cleanup();
}

unsigned int hex2dec(char *data, unsigned int len)
{
	unsigned int i;
	unsigned int value = 0;

	for (i = 0; i < len; i++) {
		value = value*0x10;
		if (data[i] >= '0' && data[i] <= '9')
			value += data[i]-'0';
		else if (data[i] >= 'A' && data[i] <= 'F')
			value += data[i]-'A' + 10;
		else
			return 0;
	}
	return value;
}

lightstone_info lightstone_get_info(lightstone dev)
{
	lightstone_info ret;
	hid_return t;
	ret.hrv = 0;
	ret.scl = 0;
	//	if (DeviceHandle != INVALID_HANDLE_VALUE)
	{
		int NumberOfBytesRead;
		char rawAscii[300];
		char InputReport[256];
		char message_started = 0;
		int char_count = 0;
		int ii;
		while(1)
		{
			t = hid_interrupt_read(dev,0x81,InputReport,0x8,10);

			if( t == HID_RET_SUCCESS )
			{
				//printf("Writing %d bytes\n", InputReport[0]+1);
				for(ii = 1; ii < InputReport[0]+1; ++ii)
				{
					if(!message_started && InputReport[ii] != '<') continue;
					message_started = 1;
					if (InputReport[ii] != 10 && InputReport[ii] != 13)
					{
						rawAscii[char_count] = InputReport[ii];
						++char_count;
					}
					else if ( InputReport[ii] == 13 ) 
					{
						rawAscii[char_count] = 0;
						if ( strlen(rawAscii) > 18) 
						{
							printf("%s\n", rawAscii);
							ret.hrv = ((float)(((hex2dec(rawAscii+5,2)) << 8) | (hex2dec(rawAscii+7,2)))) * .01;
							ret.scl = ((float)(((hex2dec(rawAscii+10,2)) << 8) | (hex2dec(rawAscii+12,2)))) * .001;
							return ret;
						}
						message_started = 0;
						char_count = 0;
					}
				}
			}
		}
	}
	return ret;
}