// �����ͨ��seam������

#pragma once

#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

string output_dir = "C:/Users/lenovo/Documents/m_�����ͼ��ѧ/Homework 3/Output/";

typedef cv::Vec3b Color;
typedef cv::Mat_<Color> Image;

const Color RED = Color(0, 0, 255);

class SeamWork
{
public:
	Image img;
	Image seam_img; // ��seam���ĵط�Ϳ�ɺ�ɫ
	vector<vector<int>> seams; // ��seam���ĵط���1
	vector<int> seam_helper_h, seam_helper_w; // ������seam�ĸ�����ÿ�е�ʣ������������
	vector<vector<int>> energy; // energy��

	vector<vector<int>> f;
	vector<vector<int>> last;

	int width, height;
	int xw, xh;
	int low_energy;

	enum ENERGY_FUNC
	{
		SOBEL, LAPLACE,
	}energy_func;

	enum DIRECTION
	{
		HORIZONTAL, VERTICAL,
	}direction;

	// Ĭ��������seam
	SeamWork(Image& input, ENERGY_FUNC choice = LAPLACE, DIRECTION d = VERTICAL) : seams(input.rows, vector<int>(input.cols, 0)),
		seam_helper_h(vector<int>(input.rows, input.cols)),
		seam_helper_w(vector<int>(input.cols, input.rows)), energy_func(choice), direction(d)
	{
		img = input.clone();
		seam_img = input.clone();
		width = xw = img.cols;
		height = xh = img.rows;

		if (d == HORIZONTAL)
		{
			Mat tmp;
			transpose(img, tmp);
			img = tmp;
			swap(xw, xh);
			swap(width, height);
		}
		calc_energy();
	}

	// Ϊ1�Ĳ�������Ҫ��remove�Ĳ���
	// ��Ϊ���¼������裺
	// ��������
	// �ο�seam������������б���ǵ����ص���������Ϊһ���ܵ͵�ֵ
	// �����seam��ɾ��
	// ����seam������ж��Ƿ����б�����ض���ɾ����
	void object_remove(vector<vector<int>> mask_remove, vector<vector<int>> mask_protect)
	{
		while (true)
		{
			// �ж��Ƿ����б�����ض���ɾ����
			bool flag = true;
			for (int i = 0; i < height; i++)
				for (int j = 0; j < width; j++)
					if (mask_remove[i][j] && !seams[i][j])
					{
						flag = false;
						break;
					}
			if (flag)
				return;

			calc_energy();
			calc_low_energy();
			// �������
			for (int i = 0; i < height; i++)
			{
				int cnt = 0;
				for (int j = 0; j < width; j++)
				{
					if (mask_protect[i][j])
						energy[i][cnt] += -low_energy * 10;
					if (!seams[i][j] && mask_remove[i][j])
						energy[i][cnt] = low_energy;
					if (!seams[i][j])
						cnt++;
				}
			}

			remove_seam(find_seam());
		}
	}

	// ��ĳ���㷨���㵱ǰ���õġ��������ֵ��
	void calc_low_energy()
	{
		low_energy = 0;
		for (int i = 0; i < xh; i++)
			for (int j = 0; j < xw; j++)
				low_energy += energy[i][j];
		low_energy = - low_energy / xw / 10;
	}

	// Ĭ��ֻɾ����
	void enlarge(int k) // ɾ��ǰk����С��seam����duplicate
	{
		// �ѱ���˵ĵط�ȫ��duplicate
		Mat ori_img(img.clone());
		for (int i = 0; i < k; i++)
		{
			calc_energy();
			remove_seam(find_seam());
		}
		Mat enlarged(height, width + k, ori_img.type());
		for (int i = 0; i < height; i++)
		{
			int cnt = 0;
			for (int j = 0; j < width; j++)
			{
				if (!seams[i][j])
					enlarged.at<Color>(i, cnt++) = ori_img.at<Color>(i, j);
				else // ���ұ߼���һ������
				{
					enlarged.at<Color>(i, cnt++) = ori_img.at<Color>(i, j);
					enlarged.at<Color>(i, cnt++) = average(ori_img.at<Color>(i, j), ori_img.at<Color>(i, j + 1));
				}
			}
		}
		img = enlarged;
	}

	// ��С
	void shrink(int k)
	{
		for (int i = 0; i < k; i++)
		{
			calc_energy();
			remove_seam(find_seam());
		}
	}

	// �ҵ�һ��seam
	int find_seam() // Ĭ��Ϊvertical
	{
		f = vector<vector<int>>(xh, vector<int>(xw, 0)); // ���������ֵ
		last = vector<vector<int>>(xh, vector<int>(xw, 0)); // ���ڼ�¼��һ�е��У�����

		cout << "looking for seam" << endl;

		for (int i = 0; i < xw; i++)
			f[0][i] = energy[0][i];

		for (int i = 1; i < xh; i++)
		{
			for (int j = 0; j < xw; j++)
			{
				int minn = 0;
				if (j > 0 && f[i-1][j-1] < f[i-1][j+minn]) minn = -1;
				if (j < xw - 1 && f[i-1][j+1] < f[i-1][j+minn]) minn = 1;
				
				last[i][j] = j + minn;
				f[i][j] = f[i-1][j+minn] + energy[i][j];
			}
		}

		int min_col = 0;
		for (int i = 0; i < xw; i++)
			if (f[xh - 1][i] < f[xh - 1][min_col])
				min_col = i;
		
		cout << min_col << endl;

		return min_col;
	}

	void remove_seam(int max_col)
	{
		// �Ƴ���ʱ�� Ϊ����ͼ��������Ȼ����Ҫ���Ƴ����ߵĵط�����ƽ���������Ƴ�����ΪP(x,y),��ô  
        // �Ƴ���P(x-1,y)������ֵΪP(x-1,y)��P(x,y)������ֵ��ƽ��  
        // P(x+1,y)������ֵΪP(x-1,y)��P(x,y)������ֵ��ƽ����Ȼ����ܰ�P(x+1,y)�ƶ���P(x,y)��λ��

		// �Ȱ�ԭͼcloneһ�ݽ��д���
		// ��ԭͼ����һ�Ŵ�С��ȷ����ͼ��������ȥ

		Mat smooth(img);
		int col = max_col;
		for (int i = xh - 1; i >= 0; i--)
		{
			// �Ƴ���(i, col)
			
			if (col > 0)
				smooth.at<Color>(i, col - 1) = average(smooth.at<Color>(i, col - 1), smooth.at<Color>(i, col));
			if (col < xw - 1)
				smooth.at<Color>(i, col + 1) = average(smooth.at<Color>(i, col + 1), smooth.at<Color>(i, col));
			
			for (int j = col; j < xw - 1; j++)
				smooth.at<Color>(i, j) = smooth.at<Color>(i, j + 1);

			mark_seam(i, col);
			col = last[i][col];
		}

		img = smooth(Rect(0, 0, xw - 1, xh)).clone();
		xw--;
	}

	void mark_seam(int row, int col)
	{
		int i, j, cnt_row, cnt_col;
		switch(direction)
		{
		case VERTICAL:
			cnt_row = -1;
			for (i = 0; i < height; i++)
			{
				if (seam_helper_h[i] >= col)
					cnt_row++;
				if (cnt_row == row) break;
			}
			cnt_col = -1;
			if (cnt_row == row)
			{	
				for (j = 0; j < width; j++)
				{
					if (seams[i][j] == 0) cnt_col++;
					if (cnt_col == col) break;
				}
				if (cnt_col == col)
				{
					seams[i][j] = 1;
					seam_img.at<Color>(i, j) = RED;
					seam_helper_h[i]--;
				}
			}
			break;
		case HORIZONTAL:
			swap(col, row);
			swap(width, height);

			cnt_col = -1;
			for (i = 0; i < width; i++)
			{
				if (seam_helper_w[i] >= row)
					cnt_col++;
				if (cnt_col == col) break;
			}
			cnt_row = -1;
			if (cnt_col == col)
			{	
				for (j = 0; j < height; j++)
				{
					if (seams[j][i] == 0) cnt_row++;
					if (cnt_row == row) break;
				}
				if (cnt_row == row)
				{
					seams[j][i] = 1;
					seam_img.at<Color>(j, i) = RED;
					seam_helper_w[i]--;
				}
			}

			swap(width, height);
			break;
		}
	}

	// ����������ɫ��ƽ��ֵ
	Color average(Color x, Color y)
	{
		return Color((x[0] + y[0]) >> 1, (x[1] + y[1]) >> 1, (x[2] + y[2]) >> 1);
	}

	void calc_energy()
	{
		switch(energy_func)
		{
		case SOBEL:
			calc_energy_sobel();
			break;
		case LAPLACE:
			calc_energy_laplace();
			break;
		default:
			calc_energy_sobel();
		}
	}

	void calc_energy_laplace()
	{
		Mat gray_img;
		cvtColor(img, gray_img, CV_BGR2GRAY); // ������ͼƬתΪ�Ҷ�ͼ���ڼ���energy
		Mat grad, laplace;
		Laplacian(gray_img, grad, CV_16S, 3, 1, 0, BORDER_DEFAULT);  
		convertScaleAbs(grad, laplace);
		
		imwrite(output_dir + "energy_laplace.png", laplace);

		xw = laplace.cols;
		xh = laplace.rows;
		energy = vector<vector<int>>(xh, vector<int>(xw, 0));
		for (int i = 0; i < xh; i++)
		{
			for (int j = 0; j < xw; j++)
				energy[i][j] = (int)laplace.at<uchar>(i, j);
		}
	}

	// ����energy��
	void calc_energy_sobel()
	{
		Mat gray_img;
		cvtColor(img, gray_img, CV_BGR2GRAY); // ������ͼƬתΪ�Ҷ�ͼ���ڼ���energy
		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y;
		Mat sobel;
		Sobel(gray_img, grad_x, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
		convertScaleAbs(grad_x, abs_grad_x);
		Sobel(gray_img, grad_y, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
		convertScaleAbs(grad_y, abs_grad_y);
		addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, sobel);

		imwrite(output_dir + "energy_sobel.png", sobel);

		xw = sobel.cols;
		xh = sobel.rows;
		energy = vector<vector<int>>(xh, vector<int>(xw, 0));
		for (int i = 0; i < xh; i++)
		{
			for (int j = 0; j < xw; j++)
				energy[i][j] = (int)sobel.at<uchar>(i, j);
		}
	}
};