#include <iostream>
#include <vector>
#include <chrono>
#include <vector>
#include <queue>

using namespace std;

struct TreeNode {
    int val;//结点值
    TreeNode *left;//左结点
    TreeNode *right;//右结点
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};


class Solution {
public:
    vector<int> ans;
    vector<int> preorderTraversal(TreeNode* root) {
        if (!root) return ans;
        dfs(root);
        return ans;
    }
    void dfs(TreeNode* root) 
    {
        if (!root) return;
        ans.push_back(root->val);//根
        dfs(root->left);//左
        dfs(root->right);//右
    }

/*
          1
      2       3
    4    5  6    7

*/

    void dfs_mid(TreeNode* root)
    {
        if (!root) return;
        dfs_mid(root->left);//左
        ans.push_back(root->val);//根
        dfs_mid(root->right);//右
    }

    void dfs_ago(TreeNode* root) 
    {
        if (!root) return;
        dfs_ago(root->left);//左
        dfs_ago(root->right);//右
        ans.push_back(root->val);//根
    }


     vector<vector<int>> levelOrder(TreeNode* root) {
        vector<vector<int>> ansm;
        if (!root) return ansm;
        queue<TreeNode*> que;
        que.push(root);
        while (!que.empty())
        {
            vector<int> path;//记录每一层的结点的值
            int n = que.size();//每一层的结点数
            while (n--)
            {
                root = que.front();
                path.push_back(root->val);
                que.pop();
                if (root->left) que.push(root->left);
                if (root->right) que.push(root->right);
            }
            ansm.push_back(path);
        }
        return ansm;
    }


};




/*
          1
      2       3
    4    5  6    7

*/
int main()
{
    TreeNode *root = new TreeNode(1, nullptr,nullptr);
    TreeNode *l = new TreeNode(2);
    TreeNode *r = new TreeNode(3);
    root->left = l;
    root->right = r;
    l->left = new TreeNode(4);
    l->right = new TreeNode(5);
    r->left = new TreeNode(6);
    r->right = new TreeNode(7);

    Solution sl;
    //sl.dfs(root);
    auto ptr = sl.levelOrder(root);
    // sl.dfs(root);
    // sl.dfs(root);

    for(int i = 0; i < ptr.size(); ++i)
    {
        cout << "+++++++++++ : " << sl.ans[i]<<endl;
        for(int j = 0; j < ptr[i].size(); ++j)
        {
            cout << "+++++++++++ : " << ptr[i][j]<<endl;
        }
    }
    return 0;
}