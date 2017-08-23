/*

Algoritmh:
  
  Gaussian Blur

Task:
  
  Create an algorithm using SSE or MMX instructions to blur an image by the Gaussian filter (use either
  inline assembler or intrisnsic functions). Try to make the function run faster than the reference
  implementation. Fill your code in the prepared regions below.

Functions:

  CustomInit() is a one-time initialization function called from the constructor of an instance of the CGaussianBlur class.

  CustomDeinit() is a one-time initialization function called from the destructor of an instance of the CGaussianBlur class.

  Blur(const unsigned char *image, int image_w, int image_h, unsigned char *new_image, float user_radius) calculates the
  Gaussian blur of a grayscale image. The image pointer points at the input image and the new_image pointer points at
  the output image. Size of both input and output images is image_w x image_h pixels. The filter works with radius equal
  to the value of user_radius parameter. Both pointers image and new_image are aligned to 16 bytes and memory for the
  output has been allocated.
  
  Project author: Martin Fajcik
  
*/

#include "gaussian_blur.h"
#include <stdio.h>
#ifndef _IPA_BENCH_

void CGaussianBlur::CustomInit()
{
	

	/*******************************************************************/
	/* TO DO: Insert your one-time initialization code here if needed. */
	/*******************************************************************/

}

void CGaussianBlur::CustomDeinit()
{


	/*************************************************************/
	/* TO DO: Insert your one-time clean up code here if needed. */
	/*************************************************************/


}

/** Reference algorithm */
/*
void CGaussianBlur::Blur_Reference(const unsigned char *image, int image_w, int image_h, unsigned char *new_image, float user_radius)
{
	float weight = 2.45f*user_radius;
	float fRadius = sqrt(weight*weight + 1);
	int radius = (int)floor(fRadius); // integer blur radius
	int x_start = radius + 1;
	int y_start = radius + 1;
	int x_end = image_w - radius;
	int y_end = image_h - radius;
	float *buffer = new float[(image_h > image_w)? image_h : image_w];	

	fRadius -= radius;                      // fraction of radius
	weight = radius*radius + fRadius*(2*radius + 1);

    for (int x = x_start; x < x_end; ++x)     // vertical blur...
    {
		float sum = image[x + (y_start - radius - 1)*image_w];
		float dif = -sum;

		for (int y = y_start - 2*radius - 1; y < y_end; ++y)
		{													// inner vertical Radius loop			
			float p = (float)image[x + (y + radius)*image_w];	// next pixel
			buffer[y + radius] = p;							// buffer pixel
			sum += dif + fRadius*p;
			dif += p;										// accumulate pixel blur

			if (y >= y_start)
			{
				float s = 0, w = 0;							// border blur correction
				sum -= buffer[y - radius - 1]*fRadius;		// addition for fraction blur
				dif += buffer[y - radius] - 2*buffer[y];	// sum up differences: +1, -2, +1

				// cut off accumulated blur area of pixel beyond the border
				// assume: added pixel values beyond border = value at border
				p = (float)(radius - y);                   // top part to cut off
				if (p > 0)
				{
					p = p*(p-1)/2 + fRadius*p;
					s += buffer[0]*p;
					w += p;
				}
				p = (float)(y + radius - image_h + 1);               // bottom part to cut off
				if (p > 0)
				{
					p = p*(p-1)/2 + fRadius*p;
					s += buffer[image_h - 1]*p;
					w += p;
				}
				new_image[x + y*image_w] = (unsigned char)((sum - s)/(weight - w)); // set blurred pixel
			}
			else if (y + radius >= y_start)
			{
				dif -= 2*buffer[y];
			}
		} // for y
    } // for x

    for (int y = y_start; y < y_end; ++y)     // horizontal blur...
    {
	    float sum = (float)new_image[x_start - radius - 1 + y*image_w];
		float dif = -sum;

		for (int x = x_start - 2*radius - 1; x < x_end; ++x)
		{                                   // inner vertical Radius loop
			float p = (float)new_image[x + radius + y*image_w];  // next pixel
			buffer[x + radius] = p;								// buffer pixel
			sum += dif + fRadius*p;
			dif += p; // accumulate pixel blur

			if (x >= x_start)
			{
				float s = 0, w = 0;               // boarder blur correction
				sum -= buffer[x - radius - 1]*fRadius; // addition for fraction blur
				dif += buffer[x - radius] - 2*buffer[x];  // sum up differences: +1, -2, +1

				// cut off accumulated blur area of pixel beyond the boarder
				p = (float)(radius - x);                   // left part to cut off
				if (p > 0)
				{
					p = p*(p-1)/2 + fRadius*p;
					s += buffer[0]*p;
					w += p;
				}
				p = (float)(x + radius - image_w + 1);               // right part to cut off
				if (p > 0)
				{
					p = p*(p-1)/2 + fRadius*p;
					s += buffer[image_w - 1]*p;
					w += p;
				}
				new_image[x + y*image_w] = (unsigned char)((sum - s)/(weight - w)); // set blurred pixel
        }
		else if (x + radius >= x_start)
		{
			dif -= 2*buffer[x];
		}
      } // x
    } // y	

	delete[] buffer;
}
*/

void CGaussianBlur::Blur(const unsigned char *image, int image_w, int image_h, unsigned char *new_image, float user_radius)
{
	__declspec(align(16)) static float _c_1[4] = {2.45f,0.0,0.0,0.0}; //TODO use __m128 _mm_set_ps(float z , float y , float x , float w );
	__declspec(align(16)) static float _c_2[1] = {user_radius};//TODO
	__declspec(align(16)) static float _c_3[4] = {1.0,0.0,0.0,0.0};
	__declspec(align(16)) static float _c_4[4] = {2.0,0.0,0.0,0.0};
	__declspec(align(16)) static float _c_5[4] = {2.0,2.0,2.0,2.0};
	__declspec(align(16)) static long _masknegint[2] = {0xFFFFFFFF,0xFFFFFFFF};
	__declspec(align(16)) static long _masknegfloat[4] = {0x80000000,0,0,0};
	__declspec(align(16)) static long _maskallnegfloat[4] = {0x80000000,0x80000000,0x80000000,0x80000000};
	__declspec(align(16)) static long _maskadd1[2] = {1,1};
	__declspec(align(16)) static long _mask2[4] = {2,0,0,0};
	__declspec(align(16)) static long _imagesize[2] = {image_w,image_h};
	__declspec(align(16)) static float tworadius;
	__declspec(align(16)) static float floatradius;
	__declspec(align(16)) static __m128 *buffer = new __m128[(image_h > image_w)? image_h : image_w];
	__declspec(align(16)) static float _nullreg1[4] = {0.0,1.0,1.0,1.0};
	__declspec(align(16)) static float _nullreg2[4] = {0.0,0.0,1.0,0.0};
	__declspec(align(16)) static float testmask[4] = {1.0,2.0,3.0,4.0};
	
	int y_start;


	__m128 c;
	//SSE
	c = _mm_mul_ss(*(__m128*)_c_1, *(__m128*)_c_2); // calculation of  weight
	__m128 weight = c;
	c = _mm_mul_ss(c,c);	//^2
	c = _mm_add_ss(c,*(__m128*)_c_3); //+1
	c = _mm_sqrt_ss(c);// = calculation of fRadius
	c = _mm_moveldup_ps(c); //copy value #1 into value #2
	_mm_store_ss(&floatradius,c); //save floating point value to avoid conversions later
	c = _mm_floor_ss(c,c); //radius
    __m128 fRadius =c;
	// MMX oprations
	__m64 radius = _mm_cvtps_pi32(c);
	__m64 startpos= radius;
	__m64 endpos= radius;
	startpos =  _mm_add_pi32(startpos,*(__m64*)_maskadd1);//add +1 x_start & y_start
	endpos = _mm_xor_si64(endpos,*(__m64*)_masknegint);//invert values 
	endpos =  _mm_add_pi32(endpos,*(__m64*)_maskadd1); // add +1 (-radius)
	endpos =  _mm_add_pi32(endpos,*(__m64*)_imagesize);//+image_w to value #1, +image_h to value #2 x_end & y_end
	// fRadius - radius
	fRadius = _mm_xor_ps(fRadius,*(__m128*)_masknegfloat); //#1 is -radius
	fRadius = _mm_hadd_ps(fRadius,fRadius);//#1 = -radius + fRadius
	__m128 sqradius = _mm_mul_ss(c,c);//#1 of sqradius is now (float)radius ^2
	c =  _mm_moveldup_ps(c); //copy value #1 into value #2
	c = _mm_mul_ss(c,*(__m128*)_c_4);//#1 of radius is now radius*2
	_mm_store_ss(&tworadius,c);
	c = _mm_add_ss(c,*(__m128*)_c_3);//#1 of radius is now (radius*2+1)
	c = _mm_mul_ss(c,fRadius);//#1 of radius is now (radius*2+1) * fRadius
	weight = _mm_add_ss(c,sqradius);// + radius^2

	__m128 s;
	__m128 w;
		/***************************************VERTICAL BLUR****************************************/
	_asm{
		push eax
		push edx
		push ecx
		;//loop init
		movq mm0,startpos
		movd eax,mm0	;//eax= x_start == MY X
		movq mm0,endpos
		movd edx,mm0    ;//edx = x_end
		xor edi,edi
		push edi
		_VERTICAL_BLUR_X:
			cmp eax,edx ;// x<x_end ?
			jnl _END_VERTICAL_BLUR_X
									;//outer loop
		;--------------------------------------------------------------------------
				xorps xmm6,xmm6
				xorps xmm7,xmm7
				xorps xmm5,xmm5

				;//float sum = image[x0,1,2,3 + (y_start - radius - 1)*image_w];
				mov ecx,image	;//note:  (y_start - radius - 1)*image_w will always be evaluated as zero
				pmovzxbd xmm5,[ecx+eax]
				
				;/*this works without SSE4
				;movd xmm5,[ecx+eax] ;//load ecx+eax..ecx+eax+3 = 4 bytes = DW = 4 values
				;pxor xmm6,xmm6
				;punpcklbw xmm5,xmm6
				;punpcklwd xmm5,xmm6	//extend those 4 values onto whole register*/
				cvtdq2ps xmm5,xmm5 ;//convert to float .. sum calculated!

				//float dif = -sum;
				movaps xmm6,xmm5			;//revert signum throught xor trick
				xorps xmm6,_maskallnegfloat		;//xmm6 = dif

				;//inner loop init
				push edx
				movq mm0,radius
				movd edx,mm0
				imul edx,2
				add edx,1 ;//extract signum b4 bracket y_start - (2*radius + 1)
				movq xmm1,startpos ;// mov startpos into SSE register	//opt can be replaced for upper mentioned thing
				movshdup xmm1,xmm1				
				movd ecx,xmm1;
				mov [y_start],ecx
				sub ecx,edx;	;//y_start - 2*radius - 1 == MY Y
				movq xmm1,endpos
				movshdup xmm1,xmm1 ;//copy #2 val into #1
				movd edx,xmm1	;//y_end
			_VERTICAL_BLUR_Y:
					cmp ecx,edx ;//y<y_end ?
					jnl _END_VERTICAL_BLUR_Y
								 	;//inner loop
			;--------------------------------------------------------------------------	
			;	/*  float p = (float)image[x0,1,2,3 + (y + radius)*image_w];	// next pixel*/
					
					push edx

					xorps xmm0,xmm0	
					xorps xmm4,xmm4
					movaps s,xmm0
					movaps w,xmm0

					movq mm0,radius
					movd edx,mm0
					add edx,ecx ;//y + radius
					push ecx
					mov ecx,image_w
					imul edx,ecx ;//*image_w
					add edx,eax ;//+x					
				
					mov edi,image
					
					pmovzxbd xmm7,[edi+edx] 
					;/* This works without SSE4
					;movups xmm7,[edi+edx] ;//load pixels
					;xorps xmm0,xmm0
					;punpcklbw xmm7,xmm0
					;punpcklwd xmm7,xmm0;	//extend those 4 values onto whole register*/
					cvtdq2ps xmm7,xmm7;//convert to float .. p calculated!
					pop ecx
						
					push eax
				;/*	buffer[y + radius] = p;							// buffer pixel*/
					movq mm0,radius
					movd edx,mm0
					add edx,ecx ;//y + radius
					mov eax,buffer
					imul edx,16
					movups [eax+edx],xmm7

				;/*sum += dif + fRadius*p;*/
				    movss xmm0,fRadius
					shufps xmm0,xmm0,0x0 ;//fradius loaded into all 4 parts
					mulps xmm0,xmm7
					addps xmm0,xmm6
					addps xmm5,xmm0


				;/*	dif += p;			*/
					addps xmm6,xmm7
					
					;//condition
					movq xmm1,startpos ;// mov startpos into SSE register	//opt can be replaced for upper mentioned thing
					movshdup xmm1,xmm1				
					movd edx,xmm1;//y_start loaded
					cmp ecx,edx ;//y >= y_start	 
					jge _OUTERIF
					jmp _ELSEIF_OUTERIF ;// else none of above is valid, jump to else

_OUTERIF:			
					xorps xmm0,xmm0
					;//float s = 0, w = 0
					movaps s,xmm0
					movaps w,xmm0

					;//sum -= buffer[y - radius - 1]*fRadius
					movq mm0,radius
					movd eax,mm0
					neg eax
					dec eax
					add eax,ecx 
					imul eax,16;//offset calculated
					mov esi,buffer
					movups xmm0,[esi+eax]
					movss xmm1,fRadius
					shufps xmm1,xmm1,0x0 
					mulps xmm0,xmm1
					subps xmm5,xmm0

					;//dif += buffer[y - radius] - 2*buffer[y];
					movq mm0,radius
					movd eax,mm0
					neg eax
					add eax,ecx
					imul eax,16;//offset calculated
					mov esi,buffer
					movups xmm0,[esi+eax]
					mov eax,ecx
					imul eax,16;//offset calc
					movups xmm1,[esi+eax]
					addps xmm1,xmm1 //*2
					subps xmm0,xmm1
					addps xmm6,xmm0

					;/*// cut off accumulated blur area of pixel beyond the border
					;// assume: added pixel values beyond border = value at border
					;p = (float)(radius - y);                   // top part to cut off */
					movq mm0,radius
					movd eax,mm0
					sub eax,ecx
					cvtsi2ss xmm7,eax ;//radius-y-1
					shufps xmm7,xmm7,0x0

					;/*if (p > 0)*/ 
					cmp eax,0
					jle _INNERIF1
					;//p = p*(p-1)/2 + fRadius*p;
					
					pcmpeqw xmm1,xmm1;//load 1
					pslld xmm1,25
					psrld xmm1,2

					movaps xmm0,xmm7
					subps xmm0,xmm1

					pcmpeqw xmm1,xmm1;//load 0.5
					pslld xmm1,26
					psrld xmm1,2

					mulps xmm0,xmm7
					mulps xmm0,xmm1

					movss xmm1,fRadius
					shufps xmm1,xmm1,0x0

					mulps xmm1,xmm7
					
					addps xmm0,xmm1
					addps xmm7,xmm3

					;/*s += buffer[0]*p*/
					mov esi,buffer
				    movups xmm0,[esi]
					mulps xmm0,xmm7;
					movaps xmm1,s
					addps xmm0,s
					movaps s,xmm1

					;//w += p;
					movaps xmm2,xmm7
					addps xmm2,w
					movaps w,xmm2
_INNERIF1:

					;//p = (float)(y + radius - image_h + 1);               // bottom part to cut off*/
					movq mm0,radius
					movd eax,mm0
					add eax,ecx
					sub eax,[image_h]
					inc eax
					cvtsi2ss xmm7,eax ;//(y + radius - image_h + 1);  
					shufps xmm7,xmm7,0x0
					
					;/*if (p > 0)*/ 
					cmp eax,0
					jle _INNERIF2
					;//p = p*(p-1)/2 + fRadius*p;
					pcmpeqw xmm1,xmm1;//load 1
					pslld xmm1,25
					psrld xmm1,2

					movaps xmm0,xmm7
					subps xmm0,xmm1

					pcmpeqw xmm1,xmm1;//load 0.5
					pslld xmm1,26
					psrld xmm1,2

					mulps xmm0,xmm7
					mulps xmm0,xmm1

					movss xmm1,fRadius
					shufps xmm1,xmm1,0x0

					mulps xmm1,xmm7
					
					addps xmm0,xmm1
					addps xmm7,xmm3

					;//s += buffer[image_h - 1]*p;
					mov esi,buffer
					mov eax,[image_h]
					dec eax
					imul eax,16
				    movups xmm0,[esi+eax]
					mulps xmm0,xmm7;
					addps xmm0,s
					movaps s,xmm1

					;//w += p;
					movaps xmm2,xmm7
					addps xmm2,w
					movaps w,xmm2
_INNERIF2:
					;//new_image[x0,1,2,3 + y*image_w] = (unsigned char)((sum - s)/(weight - w)); // set blurred pixel
					movaps xmm1,xmm5
					subps xmm1,s

					movaps xmm0,weight
					shufps xmm0,xmm0,0x0
					subps xmm0,w

					divps xmm1,xmm0

					mov esi,new_image
					mov edx,ecx	
					imul edx,image_w
					pop eax
					add edx,eax
					push eax

					cvttss2si eax,xmm1
					mov byte ptr [esi+edx], al
					shufps xmm1,xmm1,11100101b //swap first and second
					cvttss2si eax,xmm1
					mov byte ptr [esi+edx+1], al
					shufps xmm1,xmm1,11000110b //swap first and third
					cvttss2si eax,xmm1
					mov byte ptr [esi+edx+2], al
					shufps xmm1,xmm1,00100111b //swap first and fourth
					cvttss2si eax,xmm1
					mov byte ptr [esi+edx+3], al//all 4 bytes loaded into new image

					jmp _END_OUTERIF
;-----------------------------------------------------------------------------------------------------------------
_ELSEIF_OUTERIF:					
					movq mm0,radius
					movd eax,mm0
					add eax,ecx ;//y + radius
					mov edx,[y_start]
					cmp eax,edx

					jge _ELSE_OUTERIF;//y + radius >=y_start
					jmp _END_OUTERIF 
_ELSE_OUTERIF:
					;//dif -= 2*buffer[y];
					mov esi,buffer
					mov edx,ecx
					imul edx,16
				    movups xmm0,[esi+edx] 
					addps xmm0,xmm0;//*2
					subps xmm6,xmm0
					jmp _END_OUTERIF
_END_OUTERIF:
					pop eax
					pop edx
			;--------------------------------------------------------------------------
			inc ecx;//++y
			jmp _VERTICAL_BLUR_Y
			_END_VERTICAL_BLUR_Y:
			pop edx
		;--------------------------------------------------------------------------
		inc eax;//++x
		inc eax;//++x
		inc eax;//++x
		inc eax;//++x
		jmp _VERTICAL_BLUR_X
		_END_VERTICAL_BLUR_X:
		
		
		emms ;//finished working with MMX
		pop ecx
		pop edx
		pop eax
	}
		/***************************************HORIZONTAL BLUR****************************************/
	_asm{
		push eax
		push edx
		push ecx
		;//loop init
		movq xmm1,startpos ;// mov startpos into SSE register	//opt can be replaced for upper mentioned thing
		movshdup xmm1,xmm1				
		movd eax,xmm1;	;//eax= y_start == MY Y
		
		    
		movq xmm1,endpos
		movshdup xmm1,xmm1 
		movd edx,xmm1	;//edx = y_end
		
		xor edi,edi
		push edi
_HORIZONTAL_BLUR_X:
			cmp eax,edx ;// x<x_end ?
			jnl _END_HORIZONTAL_BLUR_X
									;//outer loop
		;--------------------------------------------------------------------------
				push edx
				xorps xmm6,xmm6
				xorps xmm7,xmm7
				xorps xmm5,xmm5

				;//float sum = (float)new_image[x_start - radius - 1 + y*image_w];				
				;//x_start - radius - 1 always ==0
				
				mov edx,[image_w]
				imul edx,eax
				mov esi,new_image

				pmovzxbd xmm5,[esi+edx]
				
				;/*this works without SSE4
				;movd xmm5,[ecx+eax] ;//load ecx+eax..ecx+eax+3 = 4 bytes = DW = 4 values
				;pxor xmm6,xmm6
				;punpcklbw xmm5,xmm6
				;punpcklwd xmm5,xmm6	//extend those 4 values onto whole register*/
				cvtdq2ps xmm5,xmm5 ;//convert to float .. sum calculated!


				//float dif = -sum;
				movaps xmm6,xmm5			;//revert signum throught xor trick
				xorps xmm6,_maskallnegfloat		;//xmm6 = dif

				;//inner loop init
				push edx
				movq mm0,radius
				movd edx,mm0
				imul edx,2
				inc edx ;//extract signum b4 bracket y_start - (2*radius + 1)
				
				movq mm0,startpos
				movd ecx,mm0	;//eax= x_start == MY X
				mov [y_start],ecx
				
				sub ecx,edx;	;//y_start - 2*radius - 1 == MY X

				movq mm0,endpos
				movd edx,mm0;//y_end
_HORIZONTAL_BLUR_Y:
					cmp ecx,edx ;//y<y_end ?
					jnl _END_HORIZONTAL_BLUR_Y
								 	;//inner loop
			;--------------------------------------------------------------------------	
			;	/*  float p = (float)new_image[x + radius + y*image_w];	// next pixel*/
					
					push edx

					xorps xmm0,xmm0	
					xorps xmm4,xmm4
					movaps s,xmm0
					movaps w,xmm0

					movq mm0,radius
					movd edx,mm0
					add edx,ecx ;//x + radius
				
					mov esi,[image_w]
					imul esi,eax ;//*image_			
					add edx,esi	

					push eax

					mov esi,new_image
					movzx eax,[esi+edx]
					cvtsi2ss xmm7,eax
				
					add edx,[image_w]
					shufps xmm7,xmm7,0x0
				
					movzx eax,[esi+edx]
					cvtsi2ss xmm7,eax

					add edx,[image_w]
					movsldup xmm7,xmm7

					movzx eax,[esi+edx]
					cvtsi2ss xmm7,eax

					add edx,[image_w]
					shufps xmm7,xmm7,11000100b
				
					movzx eax,[esi+edx]
					cvtsi2ss xmm7,eax
					shufps xmm7,xmm7,00100111b
						
					
				;/*	buffer[x + radius] = p;							// buffer pixel*/
					movq mm0,radius
					movd edx,mm0
					add edx,ecx ;//x + radius
					mov eax,buffer
					imul edx,16
					movups [eax+edx],xmm7

				;/*sum += dif + fRadius*p;*/
				    movss xmm0,fRadius
					shufps xmm0,xmm0,0x0 ;//fradius loaded into all 4 parts
					mulps xmm0,xmm7
					addps xmm0,xmm6
					addps xmm5,xmm0


				;/*	dif += p;			*/
					addps xmm6,xmm7
					
					;//condition
					movq xmm1,startpos ;// mov startpos into SSE register	//opt can be replaced for upper mentioned thing
					movshdup xmm1,xmm1				
					movd edx,xmm1;//y_start loaded
					cmp ecx,edx ;//y >= y_start	 
					jge _OUTERIFB
					jmp _ELSEIF_OUTERIFB ;// else none of above is valid, jump to else

_OUTERIFB:			
					xorps xmm0,xmm0
					;//float s = 0, w = 0
					movaps s,xmm0
					movaps w,xmm0

					;//sum -= buffer[y - radius - 1]*fRadius
					movq mm0,radius
					movd eax,mm0
					neg eax
					dec eax
					add eax,ecx 
					imul eax,16;//offset calculated
					mov esi,buffer
					movups xmm0,[esi+eax]
					movss xmm1,fRadius
					shufps xmm1,xmm1,0x0 
					mulps xmm0,xmm1
					subps xmm5,xmm0

					;//dif += buffer[y - radius] - 2*buffer[y];
					movq mm0,radius
					movd eax,mm0
					neg eax
					add eax,ecx
					imul eax,16;//offset calculated
					mov esi,buffer
					movups xmm0,[esi+eax]
					mov eax,ecx
					imul eax,16;//offset calc
					movups xmm1,[esi+eax]
					addps xmm1,xmm1 //*2
					subps xmm0,xmm1
					addps xmm6,xmm0

					;/*// cut off accumulated blur area of pixel beyond the border
					;// assume: added pixel values beyond border = value at border
					;p = (float)(radius - y);                   // top part to cut off */
					movq mm0,radius
					movd eax,mm0
					sub eax,ecx
					cvtsi2ss xmm7,eax ;//radius-y-1
					shufps xmm7,xmm7,0x0

					;/*if (p > 0)*/ 
					cmp eax,0
					jle _INNERIF1B
					;//p = p*(p-1)/2 + fRadius*p;
					
					pcmpeqw xmm1,xmm1;//load 1
					pslld xmm1,25
					psrld xmm1,2

					movaps xmm0,xmm7
					subps xmm0,xmm1

					pcmpeqw xmm1,xmm1;//load 0.5
					pslld xmm1,26
					psrld xmm1,2

					mulps xmm0,xmm7
					mulps xmm0,xmm1

					movss xmm1,fRadius
					shufps xmm1,xmm1,0x0

					mulps xmm1,xmm7
					
					addps xmm0,xmm1
					addps xmm7,xmm3

					;/*s += buffer[0]*p*/
					mov esi,buffer
				    movups xmm0,[esi]
					mulps xmm0,xmm7;
					movaps xmm1,s
					addps xmm0,s
					movaps s,xmm1

					;//w += p;
					movaps xmm2,xmm7
					addps xmm2,w
					movaps w,xmm2
_INNERIF1B:

					;//p = (float)(x + radius - image_w + 1);     1184          // bottom part to cut off*/
					movq mm0,radius
					movd eax,mm0
					add eax,ecx
					sub eax,[image_w]
					inc eax
					cvtsi2ss xmm7,eax ;//(y + radius - image_h + 1);  
					shufps xmm7,xmm7,0x0
					
					;/*if (p > 0)*/ 
					cmp eax,0
					jle _INNERIF2B
					;//p = p*(p-1)/2 + fRadius*p;
					pcmpeqw xmm1,xmm1;//load 1
					pslld xmm1,25
					psrld xmm1,2

					movaps xmm0,xmm7
					subps xmm0,xmm1

					pcmpeqw xmm1,xmm1;//load 0.5
					pslld xmm1,26
					psrld xmm1,2

					mulps xmm0,xmm7
					mulps xmm0,xmm1

					movss xmm1,fRadius
					shufps xmm1,xmm1,0x0

					mulps xmm1,xmm7
					
					addps xmm0,xmm1
					addps xmm7,xmm3

					;//s += buffer[image_h - 1]*p;
					mov esi,buffer
					mov eax,[image_w]
					dec eax
					imul eax,16
				    movups xmm0,[esi+eax]
					mulps xmm0,xmm7;
					addps xmm0,s
					movaps s,xmm1

					;//w += p;
					movaps xmm2,xmm7
					addps xmm2,w
					movaps w,xmm2
_INNERIF2B:
					;//new_image[x0,1,2,3 + y*image_w] = (unsigned char)((sum - s)/(weight - w)); // set blurred pixel
					movaps xmm1,xmm5
					subps xmm1,s

					movaps xmm0,weight
					shufps xmm0,xmm0,0x0
					subps xmm0,w

					divps xmm1,xmm0

					mov esi,new_image
					pop eax
					mov edx,eax	
					imul edx,image_w
					add edx,ecx
					push eax

					push ecx
					mov ecx,image_w
					cvttss2si eax,xmm1
					mov byte ptr [esi+edx], al
					shufps xmm1,xmm1,11100101b //swap first and second
					cvttss2si eax,xmm1
					add edx,ecx
					mov byte ptr [esi+edx], al
					shufps xmm1,xmm1,11000110b //swap first and third
					cvttss2si eax,xmm1
					add edx,ecx
					mov byte ptr [esi+edx], al
					shufps xmm1,xmm1,00100111b //swap first and fourth
					cvttss2si eax,xmm1		//16508
					add edx,ecx
					mov byte ptr [esi+edx], al//all 4 bytes loaded into new image
					pop ecx

					jmp _END_OUTERIFB
;-----------------------------------------------------------------------------------------------------------------
_ELSEIF_OUTERIFB:					
					movq mm0,radius
					movd eax,mm0
					add eax,ecx ;//y + radius
					mov edx,[y_start]
					cmp eax,edx

					jge _ELSE_OUTERIFB;//y + radius >=y_start
					jmp _END_OUTERIFB 
_ELSE_OUTERIFB:
					;//dif -= 2*buffer[x];
					mov esi,buffer
					mov edx,ecx
					imul edx,16
				    movups xmm0,[esi+edx] 
					addps xmm0,xmm0;//*2
					subps xmm6,xmm0
					jmp _END_OUTERIFB
_END_OUTERIFB:
					pop eax
					pop edx
			;--------------------------------------------------------------------------
			inc ecx;//++y
			jmp _HORIZONTAL_BLUR_Y
			_END_HORIZONTAL_BLUR_Y:
			pop edx
		;--------------------------------------------------------------------------
		inc eax;//++x
		inc eax;//++x
		inc eax;//++x
		inc eax;//++x
		pop edx
		jmp _HORIZONTAL_BLUR_X
		_END_HORIZONTAL_BLUR_X:
		
		
		emms ;//finished working with MMX
		pop ecx
		pop edx
		pop eax
	}
delete[] buffer;
}
#endif
