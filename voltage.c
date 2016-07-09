/* Author: Chris Oattes (cjo20)
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define BAT_FULL        4180
#define BAT_NORMAL      3680
#define BAT_LOW         3400
#define BAT_CRIT        3250
#define BAT_DEAD        2950

#define BAT_FULL_PERCENT        100
#define BAT_NORMAL_PERCENT      50
#define BAT_LOW_PERCENT         10
#define BAT_DEAD_PERCENT        0

float interpolatePercentage(float level, float above, float below, float above_percent, float below_percent)
{
        float offset = level - below;
        float range = above - below;
        float fraction = offset / range;

        float percentage_range = above_percent - below_percent;
        float result = percentage_range * fraction;
        result += below_percent;
        return result;
}

float CalculatePercentage(float level)
{
        if (level > BAT_FULL)
        {
                return 100;
        }
        else if (level > BAT_NORMAL)
        {
                return interpolatePercentage(level, BAT_FULL, BAT_NORMAL, BAT_FULL_PERCENT, BAT_NORMAL_PERCENT);
        }
        else if (level > BAT_LOW)
        {
                return interpolatePercentage(level, BAT_NORMAL, BAT_LOW, BAT_NORMAL_PERCENT, BAT_LOW_PERCENT);
        }
        else
        {
                return interpolatePercentage(level, BAT_LOW, BAT_DEAD, BAT_LOW_PERCENT, BAT_DEAD_PERCENT);
        }
}

float get_voltage(float raw)
{
	return raw * 4500.0f / 1024.0f;
}

int process_raw(char * str)
{
	char * start = strchr(str, '=');
	start += 2;

	char * end = strchr(start, '\n');
	*end = 0;

	return atoi(start);
}

float read_raw()
{
	char exec = '1';
	int i = 0;
	
	int num_samples = 0;
	int total = 0;

	for (i = 0; i < 10; ++i)
	{
		int sample = open("/sys/devices/platform/bcove_adc/basincove_gpadc/sample", O_WRONLY);

		if (sample)
		{
			int channels = open("/sys/devices/platform/bcove_adc/basincove_gpadc/channel", O_WRONLY);
			write(channels, &exec, 1);
			close(channels);
	
			write(sample, &exec, 1);
			close(sample);
	
			int results = open("/sys/devices/platform/bcove_adc/basincove_gpadc/result", O_RDONLY);
		        char data[1024];
	        	read(results, &data, 1024);
		        close(results);
        		total += process_raw(data);
			num_samples++;
		}

	}
        return total / (float)num_samples;
}

int main(int argc, char * argv[])
{
	float raw = 0.0f;
	float voltage = 0.0f;
	
	char voltage_text[33] = "voltage";
	char percentage_text[33] = "percentage";

	int mode = 0;
	if (argc > 1)
	{
		if (strcmp(argv[1], "text") == 0)
		{
			mode = 0;
		}
		else if (strcmp(argv[1], "csv") == 0)
		{
			mode = 1;
		}
		else if (strcmp(argv[1], "short") == 0)
		{
			mode = 2;
		}
		else if (strcmp(argv[1], "percentage") == 0)
		{
			mode = 3;
		}
		else if (strcmp(argv[1], "json") == 0)
		{
			mode = 4;

			if (argc == 4)
			{
				strncpy(voltage_text, argv[2], 32);
				strncpy(percentage_text, argv[3], 32);
			}
		}
		else
		{
			printf("Unknown output type: %s. Valid options:\n", argv[1]);
			printf("text: Verbose output\n");
			printf("csv: CSV values, raw reading, voltage and percentage\n");
			printf("short: Just voltage and percentage values. E.g. \"100%% 4180mV\"\n");
			printf("percentage: Single integer percentage value\n");
			printf("json: JSON output. E.g. {\"voltage\":4180, \"percentage\":100}\n");
			exit(1);
		}
	}

	raw = read_raw();
	voltage = get_voltage(raw);

	switch(mode)
	{
		case 0:
			printf("Raw Value: %.0f\nBattery Voltage: %.0fmV (%.0f%%)\n", raw, voltage, CalculatePercentage(voltage));
			break;

		case 1:
			printf("%.0f,%.0f,%.0f%%\n", raw, voltage, CalculatePercentage(voltage));
			break;

		case 2:
			printf("%.0f%% %.0fmV", CalculatePercentage(voltage), voltage);
			break;

		case 3:
			printf("%.0f", CalculatePercentage(voltage));
			break;

		case 4:
			printf("{\"%s\":%.0f, \"%s\":%.0f}", voltage_text, voltage, percentage_text, CalculatePercentage(voltage));
			break;
	}

}
