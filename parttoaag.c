#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char *argv[]){
	int l1[10000];// armazena valores de cada
	char l2[800];
	char linha[30];
	
	int atributo;
	int atributo2;
	int output;
	
	
	int inputs;
	inputs = ((argv[1][0]-'0')*10)+argv[1][1]-'0';
	if (strlen(argv[1])==3)
		inputs= (inputs*10) + (argv[1][2]-'0');
	int v = inputs+1;
	//printf("1-%s \n 2-%s\n",argv[1],argv[2]);
	int i,j;
	int t;
	int regras=0;
	int contador=0;
	
  	FILE *part;
  	FILE *aag;
	
  	char *spath= malloc (50);
  	char *dpath= malloc (50);
	
  	sprintf(spath, "tmp_iwls2020/PART/%s.part",argv[2]);
	//printf("1-%d \n 2-%s\n",inputs,spath);	
	part = fopen(spath, "r");
  	sprintf(dpath, "tmp_iwls2020/ex.aux");		
	aag = fopen(dpath, "w");
		
	//printf("%d - %d\n",inputs,inputs);
	
	i=0;
	
	while(!feof(part)){
		//printf("TESTE\n");
		fgets(linha, 30, part);
		//printf("TESTE\n");
		if (linha[0] == 'a'){
			atributo =linha[1]-'0';
			t=0;
			if (linha[2]>='0' && linha[2]<='9'){
				t=1;
				atributo = (atributo * 10)+linha[2]-'0';
				if (linha[3]>='0' && linha[3]<='9'){
					t=2;
					atributo = (atributo * 10)+linha[+3]-'0';
				}
			}
			atributo = atributo +1;
			atributo*=2;
			atributo= atributo + 1 - (linha[5+t]-'0');
			
			if(i>0){
				sprintf(l2,"\n%d %d %d",v*2,atributo2,atributo);
				fputs(l2,aag);
				atributo2= v*2;
				v++;
			}
			else
				atributo2 = atributo;
			
			i++;
			printf("l%d ",i);
			if(linha[6+t] ==':'){
				output = linha[8+t]-'0';
				
				l1[(regras)*2]=(v-1)*2;
				if(i==1){
					l1[(regras)*2]=atributo;
				}
				l1[((regras)*2)+1]=output;
				
				regras++;
				printf("regra %d = %d - %d\n",regras-1,l1[(regras-1)*2],output);
				i=0;
			}
		}
		else if (linha[0] == ':'){
			l1[(regras)*2]=linha[2]-'0';
			l1[(regras*2)+1]=2;
			regras++;
		}
	}
	
	
	//for(i=0;i<regras;i++)	
	//	printf("regra %d = %d - %d\n",i+1,l1[i*2],l1[(i*2)+1]);
	
	if(regras == 1){
		if(l1[1]==0)
			if (l1[0]=0)
				atributo2 = 1;
			else
				atributo2 = 0;
		else
			atributo2 = l1[0];
	}
	else{
		atributo2 = l1[(regras-1)*2];
		for(i=regras-2;i>=0;i--){
			atributo =l1[i*2];
			
			if(atributo%2==1)
				atributo--;
			else
				atributo++;
			if(l1[(i*2)+1]==1){
				if(atributo2%2==1)
					atributo2--;
				else
					atributo2++;
			}
			sprintf(l2,"\n%d %d %d",v*2,atributo2,atributo);
			fputs(l2,aag);
			atributo2= v*2;
			if(l1[(i*2)+1]==1)
				atributo2+=1;
			
			v++;
		}
	}
	//printf("saida = %d\n",atributo2);
	//printf("andss = %d\n",v-inputs-1);
	//printf("total = %d\n",v-1);
	
	
	fclose(part);
	fclose(aag);
	
	
	sprintf(spath, "tmp_iwls2020/ex.aux");		
	part = fopen(spath, "r");
  	sprintf(dpath, "tmp_iwls2020/AAG/%s.aag",argv[2]);	
	aag = fopen(dpath, "w");
	
	
	sprintf(linha,"aag %d %d 0 1 %d\n",v,inputs,v-1-inputs);
	fputs(linha,aag);
	for(i=2;i<=inputs*2;i+=2){
		sprintf(linha,"%d\n",i);
		fputs(linha,aag);
	}
	
	sprintf(linha,"%d\n",atributo2);
	fputs(linha,aag);
	fgets(linha,50,part);
	while(!feof(part)){
		fgets(linha,50,part);
		fputs(linha,aag);
	}
	sprintf(linha,"\n");
	fputs(linha,aag);
	fclose(part);
	fclose(aag);
	
	return 0;
}
 


