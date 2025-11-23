// 给定一个排序数组和一个目标值，在数组中找到目标值，并返回其索引。如果目标值不存在于数组中，返回它将会被按顺序插入的位置。

// 请必须使用时间复杂度为 O(log n) 的算法。

 

// 示例 1:

// 输入: nums = [1,3,5,6], target = 5
// 输出: 2
// 示例 2:

// 输入: nums = [1,3,5,6], target = 2
// 输出: 1
// 示例 3:

// 输入: nums = [1,3,5,6], target = 7
// 输出: 4

class Solution {
public:
    int searchInsert(vector<int>& nums, int target) {
        int l = 0, r = nums.size() - 1;
        while (l <= r) { //[l,r]
            int mid = l+((r-l)>>1);
            if (nums[mid] == target) {
                return mid;
            } else if (nums[mid] > target) {
                r = mid - 1;
            } else {
                l = mid + 1;
            }
        }
        return l; // l 指向的是第一个大于或等于 target 的元素的潜在位置
    }
};


// 给你一个按照非递减顺序排列的整数数组 nums，和一个目标值 target。请你找出给定目标值在数组中的开始位置和结束位置。

// 如果数组中不存在目标值 target，返回 [-1, -1]。

// 你必须设计并实现时间复杂度为 O(log n) 的算法解决此问题

class Solution {
private:
    int findBound(const vector<int>& nums, int target, bool isFindLeft) {
        int l = 0, r = nums.size() - 1;
        int index = -1;

        while (l <= r) {
            int mid = l + ((r - l) >> 1);

            if (nums[mid] == target) {
                index = mid; // only update index when there is an equal
                if (isFindLeft) { 
                    r = mid - 1;
                } else { 
                    l = mid + 1;
                }
            } else if (nums[mid] < target) {
                l = mid + 1;
            } else {
                r = mid - 1;
            }
        }
        return index;
    }

public:
    vector<int> searchRange(vector<int>& nums, int target) {
        if (nums.empty()) {
            return {-1, -1};
        }
        
        int left = findBound(nums, target, true);
        if (left == -1) {
            return {-1, -1};
        }
        int right = findBound(nums, target, false);
        
        return {left, right};
    }
};