package gkube

import (
	"reflect"
	"testing"
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
		{MergeSort([]SlotResource{}), []SlotResource{}},
		{MergeSort([]SlotResource{{
			name:                        "abc123",
			namespace:                   "test-namespace",
			resource:                    GDEPLOYMENT,
			object:                      nil,
			connectedResourceSignatures: []GSignatureConnection{},
		}}), []SlotResource{{
			name:                        "abc123",
			namespace:                   "test-namespace",
			resource:                    GDEPLOYMENT,
			object:                      nil,
			connectedResourceSignatures: []GSignatureConnection{},
		}}},
		{MergeSort([]SlotResource{{
			name:                        "abc123",
			namespace:                   "test-namespace",
			resource:                    GDEPLOYMENT,
			object:                      nil,
			connectedResourceSignatures: []GSignatureConnection{},
		}, {
			name:                        "abc123-a1b2c3",
			namespace:                   "test-namespace",
			resource:                    GPOD,
			object:                      nil,
			connectedResourceSignatures: []GSignatureConnection{},
		}, {
			name:                        "abc123-a1b2c3",
			namespace:                   "test-namespace",
			resource:                    GREPLICASET,
			object:                      nil,
			connectedResourceSignatures: []GSignatureConnection{},
		}}), []SlotResource{{
			name:                        "abc123",
			namespace:                   "test-namespace",
			resource:                    GDEPLOYMENT,
			object:                      nil,
			connectedResourceSignatures: []GSignatureConnection{},
		}, {
			name:                        "abc123-a1b2c3",
			namespace:                   "test-namespace",
			resource:                    GREPLICASET,
			object:                      nil,
			connectedResourceSignatures: []GSignatureConnection{},
		}, {
			name:                        "abc123-a1b2c3",
			namespace:                   "test-namespace",
			resource:                    GPOD,
			object:                      nil,
			connectedResourceSignatures: []GSignatureConnection{},
		}}},
	})
}
