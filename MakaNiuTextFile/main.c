#include <stdio.h>
#include <string.h>

void string2Coordinates(FILE *fp, float *vector, char *string)
{
    char look[10] = "GNSS";
    char str[500];
    char aux[4];
    int var;
    char log[20];
    int date;
    float time;
    float longitude;
    float latitude;
    char str1[100];
    char str2[100];
    while (fgets(str, 500, fp) != NULL)
    {
        var = strncmp(str, look, strlen(look));
        if (var == 0)
        {
            //Foi encontrada a string look no inicio
            sscanf(str, "%s   %d    %f    %f    %f", log, &date, &time, &longitude, &latitude);
            sscanf(str, "%s   %d    %f    %s    %s", log, &date, &time, str1, str2);
        }
    }

    vector[0] = longitude;
    vector[1] = latitude;
    strcat(str1, "  ");
    strcat(str1, str2);
    strcpy(string, str1);
    return;
}
int main()
{

    FILE *fp;
    char file_content[10000];
    char str[60];
    float coordinates[2];

    fp = fopen("sample.txt", "r");
    string2Coordinates(fp, coordinates, str);
    printf("%f %f\n", coordinates[0], coordinates[1]);
    printf("%s\n", str);

    return 0;
}