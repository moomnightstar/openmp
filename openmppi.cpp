#include<iostream>
#include<omp.h>
using namespace std;

#define NUM_THREADS 2
static long num_steps = 100000;

void cal_pi1() {
	//����
	int i;
	double x,pi, sum = 0;
	double step = 1.0 / num_steps;
	for (i = 0; i < num_steps; i++) {
		x = (i + 0.5)*step;
		sum = sum + 4.0 / (1.0 + x*x);
	}
	pi = sum*step;
	cout << "pi is " << pi << endl;
}
void cal_pi2() { //������
	int i;
	double x, pi, sum[NUM_THREADS];
	double step = 1.0 / num_steps;
	omp_set_num_threads(NUM_THREADS);
#pragma omp parallel private(i,x) shared(sum)
	{
		int  id = omp_get_thread_num();
		for (i = id, sum[id] = 0; i < num_steps; i = i + NUM_THREADS) {
			x = (i + 0.5)*step;
			sum[id] += 4.0 / (1.0 + x*x);
		}
	}
	for (i = 0, pi = 0; i < NUM_THREADS; i++) {
		pi += sum[i] * step;
	}
	cout << "pi is " << pi << endl;
}
void cal_pi3() {//�����������
	int i;
	double x, pi, sum[NUM_THREADS];
	double step = 1.0 / num_steps;
	omp_set_num_threads(NUM_THREADS);
#pragma omp parallel private(i,x) shared(sum)
	{
		int  id = omp_get_thread_num();
		sum[id] = 0;
#pragma omp for
		for (i = 0; i < num_steps; i++) {
			x = (i + 0.5)*step;
			sum[id] += 4.0 / (1.0 + x*x);
		}
	}
	for (i = 0, pi = 0; i < NUM_THREADS; i++) {
		pi += sum[i] * step;
	}
	cout << "pi is " << pi << endl;
}
void cal_pi4() {//private��critical�Ӿ�
	int i;
	double x, pi=0, sum;
	double step = 1.0 / num_steps;
	omp_set_num_threads(NUM_THREADS);
#pragma omp parallel private(i,x,sum)
	{
		int  id = omp_get_thread_num();
		for (i = id, sum = 0; i < num_steps; i = i + NUM_THREADS) {
			x = (i + 0.5)*step;
			sum += 4.0 / (1.0 + x*x);
		}
#pragma omp critical
		pi += sum*step;
	}
	cout << "pi is " << pi << endl;
}
void cal_pi5() {//���й�Լ
	int i;
	double x, pi, sum=0;
	double step = 1.0 / num_steps;
	omp_set_num_threads(NUM_THREADS);
#pragma omp parallel for reduction(+:sum) private(i,x)
	for (i = 0; i < num_steps; i++) {
		x = (i + 0.5)*step;
		sum += 4.0 / (1.0 + x*x);
	}
	pi = sum*step;
	cout << "pi is " << pi << endl;
}
int main() {
	cout << "1.�����㷨����Pi" << endl;
	cal_pi1();
	cout << "ʹ��openmp���м���pi" << endl;
	cout << "2.�ò��������Pi" << endl;
	cal_pi2();
	cout << "3.�ù�������ṹ����Pi" << endl;
	cal_pi3();
	cout << "4.��private��critical�Ӿ����Pi" << endl;
	cal_pi4();
	cout << "5.�ò��й�Լ����Pi" << endl;
	cal_pi5();
	return 0;
}
