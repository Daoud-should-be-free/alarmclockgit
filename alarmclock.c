// gcc -o alarmclock alarmclock.c -lrt
#include <signal.h> 
#include <string.h>
#include <stdio.h>
#include <time.h>
void help();
void alarmsound();
struct timespec timetowait(struct tm hour);

int main(int argc, char const *argv[]){ 
	struct sigaction actionset={.sa_flags=0};
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGRTMIN);	
	sigprocmask(SIG_BLOCK,&set,NULL);
	sigaction(SIGRTMIN,&actionset,NULL);
	
	struct tm alarm;	
	struct timespec wait={.tv_sec=300, .tv_nsec=0,};

	char flag=0;
	char message[1000]="";
	char snippet[100];	
	for(int i=1;i<argc;i++){
		if(2==sscanf(argv[i],"+%u:%u",&alarm.tm_hour,&alarm.tm_min)&&flag==0){
			wait.tv_sec=alarm.tm_hour*3600+alarm.tm_min*60;
			flag=1;
		}else if(2==sscanf(argv[i],"%u:%u",&alarm.tm_hour,&alarm.tm_min)&&alarm.tm_hour<24&&alarm.tm_min<60&&flag==0){
			wait=timetowait(alarm);
			flag=1;
		}else if(1==sscanf(argv[i],"+%um",&alarm.tm_min)&&flag==0){
			wait.tv_sec=alarm.tm_min*60;
			flag=1;
		}else if(1==sscanf(argv[i],"+%uh",&alarm.tm_hour)&&flag==0){
			wait.tv_sec=alarm.tm_hour*3600;
			flag=1;
		}else if(strcmp(argv[i],"-h")==0){
			help();
			return(0);
		}else if(1==sscanf(argv[i],"%s",snippet)){
			strcat(message,snippet);
			strcat(message," ");
		}else{
			printf("Wrong args\n");
			return(0);
		}
	}

	timer_t temp;
	
	struct sigevent event={.sigev_signo=SIGRTMIN,.sigev_notify=SIGEV_SIGNAL};
	timer_create(CLOCK_REALTIME,&event,&temp);
	struct itimerspec set_alarm={.it_value=wait,.it_interval=0};
	
	timer_settime(temp,0,&set_alarm,NULL);
	
	int null=0;
	sigwait(&set,&null);
	alarmsound(message);

	return 0;
}

struct timespec timetowait(struct tm hour){
	struct tm *moment;
	time_t(now);
	time(&now);
	moment=localtime(&now);
	long long int hours2seconds=(hour.tm_hour-moment->tm_hour);
	if (hours2seconds<0){
		hours2seconds+=24;
	}
	hours2seconds*=3600;
	long long int minutes2seconds=(hour.tm_min-moment->tm_min);

	minutes2seconds*=60;
	long long int totalsecs=hours2seconds+minutes2seconds;
	if(totalsecs<0){
		totalsecs+=24*3600;
	}

	struct timespec waitthismuch={.tv_sec=totalsecs,.tv_nsec=0,};
	
	return waitthismuch;
}
void alarmsound(char cad[1000]){
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGRTMIN);	

	timer_t temp;
	struct timespec timesound={.tv_sec=0,.tv_nsec=100000000,};
	struct itimerspec timer={.it_value=timesound,.it_interval=timesound,};
	struct sigevent event={.sigev_signo=SIGRTMIN,.sigev_notify=SIGEV_SIGNAL};

	timer_create(CLOCK_REALTIME,&event,&temp);

	timer_settime(temp,0,&timer,NULL);
	int null=0;
	for(int i=0;i<10;i++){
		if(sigwait(&set,&null)<0){
			printf("Some error happened\n");
			return ;
		};
		printf("\a");
		fflush(stdout);
	}
	printf("%s",cad);
	return;
}
void help(){
	printf("NAME\n\talarmclock\n");
	printf("SYNOPSYS\n\talarmclock [OPTIONS] \n");
	printf("DESCRIPTION\n");
	printf("\tThis is an alarm clock sloppily implemented in C. It sounds the bell and reproduces an optional message after some time.\n\n");
	printf("\tHH:MM,+HH:MM,+HHh,+MMm\n\t\tTime of alarm. Only considers the first one in the proper format. Default alarm goes off after five minutes.\n");
	printf("\t-h\n\t\tShows this help and quits\n");
	printf("\tmsg\n\t\tMessage to show\n");
	printf("MORE\n\tTo make a stable recurring alarm with this program, use crontab -e. You may want to read man crontab first");
	printf("BUGS\n\tCurrently it does not deal well with +0[h,m]|+00:00 due to the way POSIX timers are implemented\n\tOther bugs are to be expected. If found mail info to davidsotomarchena@protonmail.com\n\n\n");
}