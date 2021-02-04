// 实现双向缩放

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

typedef cv::Vec3b Color;
typedef cv::Mat_<Color> Image;

// 用于求解双向缩放图片
class BiSeamWork
{
public:
	Image img;
	Image carved_img;
	Image seam_img; // 被seam过的地方涂成红色
	vector<vector<int>> seams; // 被seam过的地方置1
	vector<vector<int>> energy; // energy阵

	// 用于求解单个seam
	vector<vector<int>> f;
	vector<vector<int>> last;
	// 用于求解最优顺序
	vector<Image> T_image[2]; // 用于在DP过程中存图片
	vector<vector<int>> T;
	vector<vector<int>> T_bc;
	// 最后可以通过T_bc找出最佳的求解顺序，重新求解一遍，再绘制seam
	vector<int> order;
	vector<vector<int>> numbered;
	int min_col_bp;

	int width, height;
	int xw, xh;

	enum ENERGY_FUNC
	{
		SOBEL, LAPLACE,
	}energy_func;

	enum DIRECTION
	{
		HORIZONTAL, VERTICAL,
	};

	BiSeamWork(Image& input, ENERGY_FUNC choice = LAPLACE) : seams(input.rows, vector<int>(input.cols, 0)), energy_func(choice)
	{
		img = input.clone();
		seam_img = input.clone();
		width = xw = img.cols;
		height = xh = img.rows;
	}

	void run(int r, int c)
	{
		T_image[0] = vector<Image>(c+1);
		T_image[1] = vector<Image>(c+1);
		T = vector<vector<int>>(r+1, vector<int>(c+1, 0));
		T_bc = vector<vector<int>>(r+1, vector<int>(c+1, -1));
		order = vector<int>(r+c);

		// 初始化第一行
		T_image[0][0] = img.clone();
		for (int i = 1; i <= c; i++)
		{
			T_image[0][i] = T_image[0][i-1].clone();
			T[0][i] = delete_seam(T_image[0][i], VERTICAL) + T[0][i-1];
			T_bc[0][i] = 1;
		}
		
		// 从第二行开始求解
        // 保存两行的图片
		for (int j = 1; j <= r; j++)
		{
			T_image[j % 2][0] = T_image[(j-1) % 2][0].clone();
			T[j][0] = delete_seam(T_image[j % 2][0], HORIZONTAL) + T[j-1][0];
			T_bc[j][0] = 1;

			for (int i = 1; i <= c; i++)
			{
				Image image_up = T_image[(j-1) % 2][i].clone();
				Image image_left = T_image[j % 2][i-1].clone();
				int up = delete_seam(image_up, HORIZONTAL) + T[j-1][i];
				int left = delete_seam(image_left, VERTICAL) + T[j][i-1];
				if (left <= up)
				{
					T_image[j % 2][i] = image_left;
					T_bc[j][i] = 1;
					T[j][i] = left;
				}
				else
				{
					T_image[j % 2][i] = image_up;
					T_bc[j][i] = 0;
					T[j][i] = up;
				}
			}
		}

		// 求解seams图
		carved_img = T_image[r % 2][c];
		int x = r, y = c, cnt = r + c - 1;
		while (x != 0 || y != 0)
		{
			order[cnt--] = T_bc[x][y];
			if (T_bc[x][y] == 1)
				y--;
			else 
				x--;
		}
		for (int i = 0; i < r+c; i++)
			cout << order[i] << endl;
		// seams=1=vertical seams=2=horizontal
		Image copy_img = img.clone();
		seam_img = img.clone();
		numbered = vector<vector<int>>(height, vector<int>(width, 0));
		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
				numbered[i][j] = i * width + j;

		// 沿原先的求解路径重新求解seam
		int yh = height, yw = width;
		for (int i = 0; i < r+c; i++)
		{
			if (order[i] == 0)
			{
				delete_seam(copy_img, VERTICAL);
				int col = min_col_bp;
				for (int j = yh - 1; j >= 0; j--)
				{
					// 移除：(j, col)
					color_seam(numbered[j][col]);
					for (int k = col; k < yw - 1; k++)
						numbered[j][k] = numbered[j][k+1];
					col = last[j][col];
				}
				yw--;
			}

			else
			{
				delete_seam(copy_img, HORIZONTAL);
				int col = min_col_bp;
				for (int j = yw - 1; j >= 0; j--)
				{
					// 移除：(col, j)	
					color_seam(numbered[col][j]);
					for (int k = col; k < yh - 1; k++)
						numbered[k][j] = numbered[k+1][j];
					col = last[j][col];
				}
				yh--;
			}
		}
	}

	void color_seam(int x)
	{
		seam_img.at<Vec3b>(x / width, x % width) = RED;
	}

	int delete_seam(Image& newimg, DIRECTION direction)
	{
		int ret;
		switch(direction)
		{
		case VERTICAL:
			calc_energy(newimg);
			ret = remove_seam_vertical(find_seam_vertical(), newimg);
			break;
		case HORIZONTAL:
			Mat tmp;
			transpose(newimg, tmp);
			newimg = tmp;
			calc_energy(newimg);
			ret = remove_seam_vertical(find_seam_vertical(), newimg);
			transpose(newimg, tmp);
			newimg = tmp;
			break;
		}
		return ret;
	}

	// 找到一个seam
	int find_seam_vertical() // 默认为vertical
	{
		f = vector<vector<int>>(xh, vector<int>(xw, 0)); // 用于求最大值
		last = vector<vector<int>>(xh, vector<int>(xw, 0)); // 用于记录上一行的列，回溯

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

		min_col_bp = min_col;
		return min_col;
	}

	int remove_seam_vertical(int max_col, Image& newimg)
	{
		Mat smooth(newimg);
		int col = max_col;
		int mov_energy = 0;
		for (int i = xh - 1; i >= 0; i--)
		{
			// 移除：(i, col)
			mov_energy += energy[i][col];
			if (col > 0)
				smooth.at<Color>(i, col - 1) = average(smooth.at<Color>(i, col - 1), smooth.at<Color>(i, col));
			if (col < xw - 1)
				smooth.at<Color>(i, col + 1) = average(smooth.at<Color>(i, col + 1), smooth.at<Color>(i, col));
			
			for (int j = col; j < xw - 1; j++)
				smooth.at<Color>(i, j) = smooth.at<Color>(i, j + 1);

			col = last[i][col];
		}

		newimg = smooth(Rect(0, 0, xw - 1, xh)).clone();
		xw--;

		return mov_energy;
	}

	// 返回两个颜色的平均值
	Color average(Color x, Color y)
	{
		return Color((x[0] + y[0]) >> 1, (x[1] + y[1]) >> 1, (x[2] + y[2]) >> 1);
	}

	void calc_energy(Image newimg)
	{
		switch(energy_func)
		{
		case SOBEL:
			calc_energy_sobel(newimg);
			break;
		case LAPLACE:
			calc_energy_laplace(newimg);
			break;
		default:
			calc_energy_sobel(newimg);
		}
	}

	void calc_energy_laplace(Image newimg)
	{
		Mat gray_img;
		cvtColor(newimg, gray_img, CV_BGR2GRAY); // 把输入图片转为灰度图用于计算energy
		Mat grad, laplace;
		Laplacian(gray_img, grad, CV_16S, 3, 1, 0, BORDER_DEFAULT);  
		convertScaleAbs(grad, laplace);
		// imwrite(output_dir + "energy_laplace.png", laplace);

		xw = laplace.cols;
		xh = laplace.rows;
		energy = vector<vector<int>>(xh, vector<int>(xw, 0));
		for (int i = 0; i < xh; i++)
		{
			for (int j = 0; j < xw; j++)
				energy[i][j] = (int)laplace.at<uchar>(i, j);
		}
	}

	// 计算energy阵
	void calc_energy_sobel(Image newimg)
	{
		Mat gray_img;
		cvtColor(newimg, gray_img, CV_BGR2GRAY); // 把输入图片转为灰度图用于计算energy
		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y;
		Mat sobel;
		Sobel(gray_img, grad_x, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
		convertScaleAbs(grad_x, abs_grad_x);
		Sobel(gray_img, grad_y, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
		convertScaleAbs(grad_y, abs_grad_y);
		addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, sobel);
		// imwrite(output_dir + "energy_sobel.png", sobel);

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