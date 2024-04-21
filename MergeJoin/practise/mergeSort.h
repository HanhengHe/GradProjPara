#include <iostream>

void Merge(int array[], int begin, int middle, int end)
{
	int n1 = middle - begin;
	int n2 = end - middle;

	int *left = new int[n1];
	int *right = new int[n2];

	for (int i = 0; i < n1; i++)
		left[i] = array[begin + i];

	for (int i = 0; i < n2; i++)
		right[i] = array[middle + i];

	int i = 0;
	int j = 0;
	int key = begin;

	for (; key < end; key++)
	{
		if (i < n1 && left[i] <= right[j]) // i stil available, left <= right, insert left
			array[key] = left[i++];
		else if (j < n2 && left[i] >= right[j]) // j still available, left > right, insert right
			array[key] = right[j++];
		else if (i == n1 && j < n2) // i reach the end (left all got inserted)
			array[key] = right[j++];
		else if (j == n2 && i < n1) // j reach the end
			array[key] = left[i++];
	}

	delete[] left;
	delete[] right;
}

void MergeSort(int Array[], int begin, int end)
{
	if (begin + 1 < end)
	{
		int middle = (end + begin) / 2;
		MergeSort(Array, begin, middle);
		MergeSort(Array, middle, end);
		Merge(Array, begin, middle, end);
	}
}

void print_array(int array[], int size)
{
	for (int i = 0; i < size; i++)
		std::cout << array[i] << " ";
	std::cout << std::endl;
}

int main()
{
	int arr[] = {12, 11, 13, 5, 6, 7};
	int arr_size = sizeof(arr) / sizeof(arr[0]);

	std::cout << "Given array is \n";
	print_array(arr, arr_size);

	MergeSort(arr, 0, arr_size);

	std::cout << "\nSorted array is \n";
	print_array(arr, arr_size);
	return 0;
}
