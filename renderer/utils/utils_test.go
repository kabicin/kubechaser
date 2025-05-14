package utils

import (
	"os"
	"reflect"
	"testing"

	"gopkg.in/yaml.v2"
	"k8s.io/apimachinery/pkg/apis/meta/v1/unstructured"
)

type Test struct {
	result   any
	expected any
}

func checkTests(t *testing.T, tests []Test) {
	for _, test := range tests {
		if !reflect.DeepEqual(test.result, test.expected) {
			t.Errorf("Error: expected %+v but the result was %+v\n", test.expected, test.result)
		}
	}
}

func Test_MergeSort(t *testing.T) {
	checkTests(t, []Test{
		{MergeSort([]int{}), []int{}},
		{MergeSort([]int{5, 3, 2, 1}), []int{1, 2, 3, 5}},
		{MergeSort([]int{5, 3, 5, 2, 1}), []int{1, 2, 3, 5, 5}},
		{MergeSort([]int{5}), []int{5}},
	})
}

func Test_MergeSortString(t *testing.T) {
	checkTests(t, []Test{
		{MergeSortString([]string{}), []string{}},
		{MergeSortString([]string{"a", "c", "d", "b"}), []string{"a", "b", "c", "d"}},
		{MergeSortString([]string{"e", "b", "a", "c", "d"}), []string{"a", "b", "c", "d", "e"}},
		{MergeSortString([]string{"a"}), []string{"a"}},
	})
}

func loadTestFileAsUnstructured(fileName string, t *testing.T) *unstructured.Unstructured {
	podBytes, err := os.ReadFile(fileName)
	if err != nil {
		t.Errorf("loadTestFile: Could not read test file %s", fileName)
	}

	pod := &unstructured.Unstructured{}
	err = yaml.Unmarshal(podBytes, &pod.Object)
	if err != nil {
		t.Error(err)
		t.Errorf("loadTestFile: Could not unmarshal yaml for file %s", fileName)
	}
	return pod
}

func Test_CreateOrderedMap(t *testing.T) {
	pod := loadTestFileAsUnstructured("../../tests/pod.yaml", t)
	obj1, obj1hash := CreateOrderedMap(pod.Object)
	obj2, obj2hash := CreateOrderedMap(pod.Object)
	checkTests(t, []Test{
		{obj1.Equals(obj2), true},
		{obj1hash, obj2hash},
	})
}

func Test_DuplicateOrderedMap(t *testing.T) {
	pod := loadTestFileAsUnstructured("../../tests/pod.yaml", t)
	obj1, obj1hash := CreateOrderedMap(pod.Object)
	obj2, obj2hash := DuplicateOrderedMap(obj1)
	checkTests(t, []Test{
		{obj1.Equals(obj2), true},
		{obj1hash, obj2hash},
	})
}
