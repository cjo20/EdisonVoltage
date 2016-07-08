/* Author: Chris Oattes (cjo20)
This is incredibly hacky and requires fixing
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#define BAT_FULL        4180
#define BAT_NORMAL      3680
#define BAT_LOW         3400
#define BAT_CRIT        3250
#define BAT_DEAD        3200

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

	for (i = 0; i < 5; ++i)
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
	char buf[1024];
	buf[0] = 0;
	printf("Battery voltage monitoring\n");

	char * trigger_path = "/tmp/battery_trigger";	
	char * voltage_path = "/tmp/battery_voltage";
	char * percentage_path = "/tmp/battery_percentage";
	char * raw_path = "/tmp/battery_raw";

	dev_t dev = 0;

	unlink(trigger_path);

	int status = mkfifo(trigger_path, 0777);
	status = mknod(voltage_path, 0777, dev);
	status = mknod(percentage_path, 0777, dev);
	status = mknod(raw_path, 0777, dev);


	while (buf[0] != '9')
	{
		char output[7];
		float voltage = 0.0f;
		float raw = 0;

		chmod(trigger_path, S_IWUSR | S_IRUSR | S_IWGRP | S_IWOTH);
		int fd = open(trigger_path, O_RDONLY);
		read(fd, buf, sizeof(buf));
		close(fd);


		if (buf[0] == '9')
		{
			break;
		}
		printf("Got trigger\n");		
		raw = read_raw();
		voltage = get_voltage(raw);
		
		snprintf(output, 7, "%.0f", voltage);
		fd = open(voltage_path, O_WRONLY);
		write(fd, output, strlen(output) + 1);
		close(fd);

		snprintf(output, 7, "%.0f", CalculatePercentage(voltage));
		fd = open(percentage_path, O_WRONLY);
		write(fd, output, strlen(output) + 1);
		close(fd);	

		snprintf(output, 7, "%.0f", raw);
		fd = open(raw_path, O_WRONLY);
		write(fd, output, strlen(output) + 1);
		close(fd);		

		printf("Done\n");
	}

	unlink(trigger_path);
	unlink(voltage_path);
	unlink(percentage_path);
	unlink(raw_path);
}
