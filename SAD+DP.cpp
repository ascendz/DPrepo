#include <cstdio>
#include <cstring>
#include <iostream>
#include<cv.h>
#include<highgui.h>
#include <cmath>
#include<time.h>
using namespace std;
const int Width =  512;
const int Height = 512;
int Ddynamic[Width][Width];

int  main()
{  
    //打开文件
    FILE* pFile;
    if (pFile = fopen("data.txt","a"))
    {
        //cout <<"File Open Success"<<endl; 
    }else{
        //cout <<"File Open Failed"<<end;
        return 0;
    }
    // IplImage * leftImage = cvLoadImage("l1.png",0);
    // IplImage * rightImage = cvLoadImage("r1.png",0);

    //IplImage * leftImage = cvLoadImage("l2.jpg",0);
    //IplImage * rightImage = cvLoadImage("r2.jpg",0);


    IplImage * leftImage = cvLoadImage("imL.png",0);                           
    IplImage * rightImage = cvLoadImage("imR.png",0);

    int imageWidth = leftImage->width;
    int imageHeight =leftImage->height;

    IplImage * leftDepth = cvCreateImage(cvGetSize(leftImage),leftImage->depth,1);
    IplImage * rightDepth = cvCreateImage(cvGetSize(leftImage),leftImage->depth,1);

    IplImage * leftValid = cvCreateImage(cvGetSize(leftImage),leftImage->depth,1);
    IplImage * rightValid = cvCreateImage(cvGetSize(leftImage),leftImage->depth,1);

    IplImage * leftFilter = cvCreateImage(cvGetSize(leftImage),leftImage->depth,1);
    IplImage * rightFilter = cvCreateImage(cvGetSize(leftImage),leftImage->depth,1);

    IplImage * depth = cvCreateImage(cvGetSize(leftImage),leftImage->depth,1);
    IplImage * valid = cvCreateImage(cvGetSize(leftImage),leftImage->depth,1);

    unsigned char * pPixel = NULL;
    unsigned char  pixel;
    unsigned char * pPixel2 = NULL;
    unsigned char  pixel2;
    cvZero(leftDepth);
    cvZero(rightDepth);
    cvZero(leftValid);
    cvZero(rightValid);
    cvZero(leftFilter);
    cvZero(rightFilter);
    cvZero(depth);
    cvZero(valid);



    CvSize SADWindowSize = cvSize(5,5);  //(widh,height)
    int SW2 = SADWindowSize.width/2;
    int SH2 = SADWindowSize.height/2;
    cout<<"SH2:  "<<SH2<<endl;
    cout<<"SW2:  "<<SW2<<endl;

    int minD = 0;
    int maxD = 15;
    //假设图像是经过矫正的，那么每次都只是需要搜搜同一行的内容
    int max12Diff = 8*SADWindowSize.width*SADWindowSize.height;

    for (int i = 0;i < imageWidth;i++)
    {
        Ddynamic[0][i] = 0;
        Ddynamic[i][0] = 0;
    }
    unsigned char * pLeftPixel  = NULL;
    unsigned char * pRightPixel = NULL;
    unsigned char leftPixel = 0;
    unsigned char rightPixel =0;
    int m,n,l;
    int difPixel = 0;
    int t1 = clock();
    for ( i = SH2 ; i < imageHeight -SH2;i++)
    {
        for (int j = SW2; j<imageWidth - SW2;j++)
        {
            for (int k = j + minD; k <= j + maxD;k++)
            {
                difPixel = 0;
                for (m = -SH2;m <= SH2;m++)
                {
                    for (n = -SW2; n <= SW2;n++)
                    {
                        pRightPixel= (unsigned char*)rightImage->imageData+
                            (i+m)*rightImage->widthStep + j+n;
                        rightPixel = *pRightPixel;
                        if (k < SW2  || k  >= imageWidth -SW2 )
                        {
                            difPixel += rightPixel;
                        }else {
                            pLeftPixel = (unsigned char*)leftImage->imageData + 
                                (i+m)*leftImage->widthStep + k+n;  
                            leftPixel  = *pLeftPixel; 
                            difPixel += abs(leftPixel - rightPixel);
                        }

                    }
                }
                if (difPixel <= max12Diff)
                {
                    Ddynamic[j + 1][k + 1] = Ddynamic[j][k] +1; 
                }else if (Ddynamic[j][k+1] > Ddynamic[j+1][k])
                {
                    Ddynamic[j + 1][k + 1] = Ddynamic[j][k+1];
                }else{
                    Ddynamic[j+1][k+1] = Ddynamic[j+1][k];
                }

                //cout<<Ddynamic[j +1][k+1]<<"  ";
            }
            //cout<<"\n";
        }
        //逆向搜索，找出最佳路径
        m = imageWidth - SW2;
        n = imageWidth - SW2;
        int m2 = m, n2 = n;

        while( m >= 1 && n >= 1)
        {
            if ((m2 == m + 1 && n2 >= n +1) || ( m2 > m +1 && n2 == n + 1))
            {
                pPixel = (unsigned char *)rightDepth->imageData + i*rightDepth->widthStep + m;
                *pPixel = (n-m)*10;
                //标记有效匹配点
                pPixel = (unsigned char *)rightValid->imageData + i*rightValid->widthStep + m;
                *pPixel = 255;

                m2 = m;
                n2 = n;
            }
            if (Ddynamic[m-1][n-1] >= Ddynamic[m][n -1] && Ddynamic[m-1][n -1] >= Ddynamic[m-1][n])
            {

                m--; 
                n--;
            }else if (Ddynamic[m-1][n] >= Ddynamic[m][n -1] && Ddynamic[m-1][n] >= Ddynamic[m-1][n -1])
            { 
                m--;
            }
            else
            { 
                n--;
            }

        }

        //cvWaitKey(0);
    }
    int t2 = clock();
    cout<<"dt: "<<t2-t1<<endl;
    //统计未能匹配点的个数
    int count = 0;
    for ( i = 0 ;i< imageHeight;i++)
    {
        for (int j= 0; j< imageWidth;j++)
        {
            pPixel = (unsigned char *)rightValid->imageData + i*rightValid->widthStep + j;
            pixel = *pPixel;
            if (pixel == 0)
            {
                count++;
            }
        }
    }
    cout<<"rightCount:  "<<count<<"  "<<(double)count/(imageWidth*imageHeight)<<endl;
    rightFilter = cvCloneImage(rightDepth);
    //7*7中值滤波
    int halfMedianWindowSize = 3;
    int medianWindowSize = 2*halfMedianWindowSize + 1;
    int medianArray[100] = {0};
    count = 0;
    int temp = 0;
    int medianVal = 0;

    for ( i = halfMedianWindowSize + 1 ;i< imageHeight - halfMedianWindowSize;i++)
    {
        for (int j = halfMedianWindowSize; j< imageWidth - halfMedianWindowSize;j++)
        {
            pPixel = (unsigned char *)rightValid->imageData + i*rightValid->widthStep + j;
            pixel = *pPixel;
            if (pixel == 0)
            {
                count = 0;
                for (int m = i - halfMedianWindowSize ; m <= i + halfMedianWindowSize ;m++)
                {
                    for (int n = j - halfMedianWindowSize; n <= j + halfMedianWindowSize ;n++)
                    {
                        pPixel2 = (unsigned char *)rightDepth->imageData + m*rightDepth->widthStep + n;
                        pixel2 = *pPixel2;
                        if (pixel2 != 0)
                        {
                            medianArray[count] = pixel2;
                            count++;
                        }

                    }
                    //排序
                    for (int k = 0; k< count;k++)
                    {
                        for (int l = k + 1; l< count;l++)
                        {
                            if (medianArray[l] < medianArray[l-1] )
                            {
                                temp = medianArray[l];
                                medianArray[l] = medianArray[l-1];
                                medianArray[l-1] = temp;
                            }
                        }
                    }
                    medianVal = medianArray[count/2];
                    pPixel = (unsigned char *)rightFilter->imageData + i*rightFilter->widthStep + j;
                    *pPixel = medianVal;
                }

            }
        }
    }

    //两次进行DP计算，分别是左对右和右对左,从右想左扫描
    for (int j = 0; j<= imageWidth;j++)
    {
        for (int k = 0;k <= imageWidth;k++)
        {
            Ddynamic[j][k] = 0;
        }
    }
    minD = -maxD;
    maxD = 0;
  
    for ( i = SH2 ; i < imageHeight -SH2;i++)
    {
        for (int j = imageWidth-SW2-1; j >= SW2;j--)
        {
            for (int k = j + maxD; k >= j + minD;k--)
            {
                difPixel = 0;
                for (m = -SH2;m <= SH2;m++)
                {
                    for (n = -SW2; n <= SW2;n++)
                    {
                        pLeftPixel = (unsigned char*)leftImage->imageData + 
                                     (i+m)*leftImage->widthStep + j+n;
                        leftPixel  = *pLeftPixel;
                        if (k < SW2 || k  >= imageWidth -SW2)
                        {
                             difPixel += leftPixel;
                        }else {
                            
                            pRightPixel= (unsigned char*)rightImage->imageData+
                                         (i+m)*rightImage->widthStep + k+n; 
                            rightPixel = *pRightPixel;
                            difPixel += abs(leftPixel - rightPixel);
                            //cout<<Ddynamic[j +1][k+1]<<"  ";
                        }
                    }
                }

                if (difPixel <= max12Diff)
                {
                    Ddynamic[j][k] = Ddynamic[j+1][k+1] +1; 
                }else if (Ddynamic[j+1][k] > Ddynamic[j][k+1])
                {
                    Ddynamic[j][k] = Ddynamic[j+1][k];
                }else{
                    Ddynamic[j][k] = Ddynamic[j][k+1];
                }

            }
            //cout<<"\n";
        }
        //逆向搜索，找出最佳路径

        m = 0;
        n = 0;
        int m2 = m, n2 = n;
        while( m < imageWidth && n < imageWidth)
        {
            if ((m == m2 + 1 && n >= n2 +1) || ( m > m2 +1 && n == n2 + 1))
            {
                pPixel = (unsigned char *)leftDepth->imageData + i*leftDepth->widthStep + m;
                *pPixel = (m-n)*10;
                //标记有效匹配点
                pPixel = (unsigned char *)leftValid->imageData + i*leftValid->widthStep + m;
                *pPixel = 255;

                m2 = m;
                n2 = n;
            }
            if (Ddynamic[m+1][n+1] >= Ddynamic[m][n+1] && Ddynamic[m+1][n+1] >= Ddynamic[m+1][n])
            {

                m++; 
                n++;
            }else if (Ddynamic[m+1][n] >= Ddynamic[m][n +1] && Ddynamic[m+1][n] >= Ddynamic[m+1][n+1])
            { 
                m++;
            }
            else
            { 
                n++;
            }

        }
        //cvWaitKey(0);
    }
    int t3 = clock();
    //refine the depth image  7*7中值滤波
    //统计未能匹配点的个数
    count = 0;
    for ( i = 0 ;i< imageHeight;i++)
    {
        for (int j= 0; j< imageWidth;j++)
        {
            pPixel = (unsigned char *)leftValid->imageData + i*leftValid->widthStep + j;
            pixel = *pPixel;
            if (pixel == 0)
            {
                count++;
            }
        }
    }

    cout<<"Left Count:  "<<count<<"  "<<(double)count/(imageWidth*imageHeight)<<endl;

    //将关键信息计入文件
    fprintf(pFile,"\nDP:\nClock:   %dms",t3-t1);
    fprintf(pFile,"\nInvalid Point:   %f",(double)count/(imageWidth*imageHeight));
    // cvWaitKey(0);
    leftFilter = cvCloneImage(leftDepth);
    //7*7中值滤波
    count = 0;
    for ( i = halfMedianWindowSize + 1 ;i< imageHeight - halfMedianWindowSize;i++)
    {
        for (int j = halfMedianWindowSize; j< imageWidth - halfMedianWindowSize;j++)
        {
            pPixel = (unsigned char *)leftValid->imageData + i*leftValid->widthStep + j;
            pixel = *pPixel;
            if (pixel == 0)
            {
                count = 0;
                for (int m = i - halfMedianWindowSize ; m <= i + halfMedianWindowSize ;m++)
                {
                    for (int n = j - halfMedianWindowSize; n <= j + halfMedianWindowSize ;n++)
                    {
                        pPixel2 = (unsigned char *)leftDepth->imageData + m*leftDepth->widthStep + n;
                        pixel2 = *pPixel2;
                        if (pixel2 != 0)
                        {
                            medianArray[count] = pixel2;
                            count++;
                        }

                    }
                    //排序
                    for (int k = 0; k< count;k++)
                    {
                        for (int l = k + 1; l< count;l++)
                        {
                            if (medianArray[l] < medianArray[l-1] )
                            {
                                temp = medianArray[l];
                                medianArray[l] = medianArray[l-1];
                                medianArray[l-1] = temp;
                            }
                        }
                    }
                    medianVal = medianArray[count/2];
                    pPixel = (unsigned char *)leftFilter->imageData + i*leftFilter->widthStep + j;
                    *pPixel = medianVal;
                }

            }
        }
    }
    //假如已知左右图片的深度，进行refinement
    unsigned char dLeft = 0, dRight = 0;
    count = 0;
    for ( i = 0; i< imageHeight;i++)
    {
        for (int j = 0; j< imageWidth;j++)
        {
            pRightPixel= (unsigned char*)rightDepth->imageData + i*rightDepth->widthStep + j;
            dRight = (*pRightPixel)/10;
            if (j + dRight < imageWidth)
            {    
                pPixel= (unsigned char*)depth->imageData + i*depth->widthStep + j;
                pLeftPixel= (unsigned char*)leftDepth->imageData + i*leftDepth->widthStep + j + dRight;
                dLeft = (*pLeftPixel)/10;
                if (abs(int(dRight) -(int)dLeft) <= 1)
                {
                    //深度一致  
                    *pPixel = dRight*10;
                    
                }else{
                    //深度不一致，认为是误匹配、错误匹配
                    *pPixel = 0;
                    count++;
                }
            }
        }
    }
    cout<<"depth Count:  "<<count<<"  "<<(double)count/(imageWidth*imageHeight)<<endl;


    cvNamedWindow("leftImage",1);
    cvNamedWindow("rightImage",1);
    cvNamedWindow("leftDepth",1);
    cvNamedWindow("leftValid",1);
    cvNamedWindow("rightDepth",1);
    cvNamedWindow("rightValid",1);
    cvNamedWindow("leftFilter",1);
    cvNamedWindow("rightFilter",1);
    cvNamedWindow("depth",1);

    
    cvShowImage("leftImage",leftImage);
    cvShowImage("rightImage",rightImage);

    cvShowImage("leftDepth",leftDepth);
    cvShowImage("rightDepth",rightDepth);

    cvShowImage("leftValid",leftValid); 
    cvShowImage("rightValid",rightValid);

    cvShowImage("leftFilter",leftFilter);
    cvShowImage("rightFilter",rightFilter);
    cvShowImage("depth",depth);

    cvSaveImage("leftDepth.jpg",leftDepth);
    cvSaveImage("rightDepth.jpg",rightDepth);
    cvSaveImage("leftValid.jpg",leftValid);
    cvSaveImage("rightValid.jpg",rightValid);
    cvSaveImage("leftFilter.jpg",leftFilter);
    cvSaveImage("rightFilter.jpg",rightFilter);
    cvSaveImage("depth.jpg",depth);



    fclose(pFile);
    cvWaitKey(0);
    return 0;
}