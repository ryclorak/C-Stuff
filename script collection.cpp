///random c/c++ scripts

using namespace std;

int Fact(int num)
{
  int sum=1;
  for (i=num; i>0; i--)
    sum*=num;
  return sum;
}




void ReadFile(string file)
{
  ifstream ifs;
  int inp;

  cout << "Reading file" << file << "... ";
  ifs.open(file);
  if (!ifs)
    cout << "Failed opening " << file << " for read." << endl;
  else
  {
    while (!ifs.eof())
    {
      ifs >> inp;
    }
  }
  ifs.close();
  cout << "File closed." << endl;
}




void WriteFile(string file, int val)
{
  ofstream ofs;
  
  cout << "Writing to file " << file << "... ";
  ofs.open(file);
  if(!ofs)
    cout << "Failed opening " << file << " for write." << endl;
  else
    {
      ofs << val;
    }

  ofs.close();
  cout << "Done writing." << endl;
}




void PrintArray(int arr[], int c)
{
  for (int i=0; i<c; i++)
    cout << arr[i] << " ";
}





void PreOrder(Node* root)
{
  if (root)
  {
    cout << root->val << " ";
    PreOrder(root->left);
    PreOrder(root->right);
  }
}





void InOrder(Node* root)
{
  if (root)
  {
    InOrder(root->left);
    cout << root->val << " ";
    InOrder(root->right);
  }
}




void PostOrder(Node* root)
{
	if (root)
	{
		PostOrder(root->left);
		PostOrder(root->right);
		cout << root->val << " ";
	}
}
