#include<stdio.h>
#include<stdlib.h>

void sort(float *address, int q)
{
	int m=0,n=0;
	float temp;
	for(;n<q;n++)
		for(m=0;m<q-1;m++)
			if(*(address+m)>=*(address+m+1))
			{
				temp=*(address+m);
				*(address+m)=*(address+m+1);
				*(address+m+1)=temp;
			}
}

int main()
{
	int k;
	int *i=(int*)malloc(sizeof(int));
	scanf("%d",i);
	float *array=(float*)malloc((*i)*sizeof(float));
	for(k=0;k<*i;k++)
		scanf("%f",array+k);
	sort(array, *i);
	for(int j=0;j<*i;j++)
		printf("%f  ",array[j]);
	
	free(i);
	free(array);
	getchar();
	getchar();
	
	return 0;
	
}