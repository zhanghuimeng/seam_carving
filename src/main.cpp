#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "IntWindow.h"
#include "Seam.h"
#include "BiSeamWork.h"

using namespace std;
using namespace cv;

typedef Vec3b Color;
typedef Mat_<Color> Image;

int main()
{
	Image img;

	img = imread("C:/Users/lenovo/Documents/m_�����ͼ��ѧ/Homework 3/Images/2.png");

	// Image img_remove = img.clone(); // ����һ��ͼ������ʾ����Ϊ��Ҫ����ʾ��ͼ���������ע���Ӷ�����޸ģ�
	// vector<vector<int>> mask_remove(getMaskRemove(img_remove, img_remove.rows, img_remove.cols));
	// vector<vector<int>> mask_protect(getMaskProtect(img_remove, img_remove.rows, img_remove.cols));

	// Mat gray_img;
	// cvtColor(img, gray_img, CV_BGR2GRAY);

	SeamWork work_enlarge(img, SeamWork::SOBEL, SeamWork::VERTICAL);
	SeamWork work_shrink(img, SeamWork::SOBEL, SeamWork::VERTICAL);
	// SeamWork work_remove(img, SeamWork::LAPLACE, SeamWork::VERTICAL);

	work_enlarge.enlarge(100);
	work_shrink.shrink(100);
	// work_remove.object_remove(mask_remove, mask_protect);

	imwrite(output_dir + "enlarged.png", work_enlarge.img);
	imwrite(output_dir + "seams_enlarged.png", work_enlarge.seam_img);
	imwrite(output_dir + "shrinked.png", work_shrink.img);
	imwrite(output_dir + "seams_shrinked.png", work_shrink.seam_img);
	// imwrite(output_dir + "removed.png", work_remove.img);
	// imwrite(output_dir + "seams_removed.png", work_remove.seam_img);
	// imwrite(output_dir + "mask.png", img_remove);

	BiSeamWork work_optimal_order(img);
	work_optimal_order.run(10, 10);
	imshow("carved", work_optimal_order.carved_img);
	imwrite(output_dir + "bi_carved.png", work_optimal_order.carved_img);
	imwrite(output_dir + "bi_seam.png", work_optimal_order.seam_img);
	waitKey(0);
	return 0;
}
