#include<iostream>
#include<cmath>
#include<string>
#include<omp.h>
#include<cstdlib>
#include<ctime>
using namespace std;
int w;
class list {
	public:  double *data;
			 int length;  
	list(int n) {
			length = n;
			data = new double[n];
		}
	~list() {

	}
	void output() {
		for (int i = 0; i < length; i++) {
			
			cout << data[i] << ' ';
			if ((i+1)%w== 0) cout << endl;
		}
	}
	void sort(int m, int n) { //从m开始对m个数进行局部串行排序
		double temp;
		if ((m + n )> length) n = length-m;
		for (int i = 0; i < n-1; i++) {
			for (int j = i + 1; j < n;j++) {
				if (data[m + j] < data[m + i]) {
					temp=data[m+i];
					data[m + i] = data[m + j];
					data[m + j] = temp;
				}
			}
		}
	}
	
};
void merge(double b[], int p, int q, int r) {
	double *temp;
	temp = new double[r-p+1];
	int i = p, j = q + 1;
	int m = q, n = r;
	int k = 0;
	while (i <= m&&j <= n) {
		if (b[i] <= b[j])  temp[k++] = b[i++];
		else temp[k++] = b[j++];
	}
	while (i <= m)
		temp[k++] = b[i++];

	while (j <= n)
		temp[k++] = b[j++];

	for (i = 0; i < k; i++)
		b[p + i] = temp[i];
	free(temp);
}
void mergesort(double b[], int p, int r) { //归并排序
	if (p < r) {
		int q = (p + r) / 2;
		mergesort(b, p, q);
		mergesort(b, q + 1, r);
		merge(b, p, q, r);
	}
}

int main() {
	int n,p,tid;
	cout << "input the list length and thread numble"<<endl;
	cin >> n>>p;
	w = n / p;  //均匀划分
	list List(n);
	list Sample(p*p);

	srand((unsigned)time(NULL));
	for (int i=0; i < n; i++) {
		List.data[i] = rand()%100+1;
	}
	cout << "a.均匀划分" << endl;
	List.output();

	omp_set_num_threads(p);
#pragma omp parallel shared(List) private(tid)
	{
		tid = omp_get_thread_num();
		List.sort(tid*w, w);    //局部排序
	}
	cout << "b.局部排序" << endl;
	List.output();

#pragma omp parallel shared(List,p,Sample) private(tid)
	{
		tid = omp_get_thread_num();
		for (int i = 0; i < p; i++) {
			Sample.data[tid*p + i] = List.data[tid*w + i*p];
		}
	}
	cout << "c.正则采样" << endl;
	Sample.output();
	Sample.sort(0, Sample.length);
	cout << "d.样本排序" << endl;
	Sample.output();

	cout << "e.选择主元" << endl;

	list elem(p - 1);

	for (int i=0; i < p - 1; i++) {
		elem.data[i] = Sample.data[i*p+p];
	}
	elem.output();
	//主元划分
	int **send_buf= new int*[p];   //设置send_buf[][],用来记录每个处理中数据段的划分位置,第一维表示处理器，第二维表示第0至第p个划分位置
	for (int i=0; i < p; i++) {
		send_buf[i] = new int[p + 1];
		for (int j=0; j < p + 1; j++) {
			send_buf[i][j] = 0;
		}
	}
	
#pragma omp parallel shared(List,w,elem,send_buf) private(tid)
	{
		tid = omp_get_thread_num();
		send_buf[tid][0] = 0;   //第一划分位都为0
		send_buf[tid][p] = w;   //最后划分为都为段截止处+1；(w=n/p)  
		int  i = 1, j = 0;
		while (i < p&&j < w) {
			if (List.data[tid*w + j] > elem.data[i-1]) {
				send_buf[tid][i] = j; //若有序段中数大于主元；则设划分点为该数
				i++; //寻找下一个划分点
			}
			else j++;
		}
		while (i < p) {
			send_buf[tid][i] = w;//将剩下的划分点全部置为w
			i++;
		}
	}
	//通过以上计算，找到每一个划分段在数据中的位置为：[send_buf[tid][i],sendbuf[tid][i+1]) ,相减可计算段长度
	int *length;
	length = new int[p];
	double *rec_buf;
#pragma omp parallel shared(List,w,send_buf,length) private(tid,rec_buf)//给每一个分配接收缓存
	{
		tid = omp_get_thread_num();
		length[tid] = 0;
		for (int i=0; i < p; i++) {
			length[tid] += send_buf[i][tid + 1] - send_buf[i][tid];  //记录每个处理器要接收的数据长度
		}
		rec_buf = new double[length[tid]];
		int k=0;
		for (int i=0; i < p; i++) {
			for (int j = send_buf[i][tid ]; j < send_buf[i][tid+1]; j++) {
				rec_buf[k] = List.data[i*w + j]; k++;  //从原数据List中接收数据到rec_buf中
			}
		}
		mergesort(rec_buf, 0, length[tid] - 1);//每一个处理器开始进行归并排序
#pragma omp barrier  //设置同步路障，等待以上计算完成
		{
			int sum = 0;
			for (int i = 0; i < tid; i++) {
				sum += length[i];  //计算每一个处理器送回数据的总长度
			}
			for (int i = 0; i < length[tid]; i++) {
				List.data[sum + i] = rec_buf[i];  //原序列接受每一个处理器排序的结果
			}
		}
	}	
	cout << endl << "h.排序结果" << endl;
	List.output();
	return 0;
}
