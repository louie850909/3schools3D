#include "file.h"

INSTANCE g_GrassSaveData[MAX_GRASS];
INSTANCE g_TreeSaveData[MAX_TREE];
int g_checkSum;

void SaveData(int save_target)
{
	FILE* fp;
	char* adr;
	int check_sum;

	INSTANCE* grass = getGrassInstance();
	INSTANCE* tree = getTreeInstance();
	
	switch (save_target)
	{
	case SAVE_GRASS:
		for (int i = 0; i < MAX_GRASS; i++)
		{
			g_GrassSaveData[i] = grass[i];
		}

		// セーブデータのチェックサムを求める
		adr = (char*)&g_GrassSaveData;	// デーブデータの先頭アドレスをadrに入れる
		check_sum = 0;
		g_checkSum = 0;
		for (int i = 0; i < sizeof(g_GrassSaveData); i++)
		{
			check_sum += adr[i];
		}
		g_checkSum = check_sum;

		// ファイルに出力
		fp = fopen("data/BIN/grass.bin", "wb");

		fwrite(&g_GrassSaveData, 1, sizeof(g_GrassSaveData), fp);
		fwrite(&g_checkSum, 1, sizeof(g_checkSum), fp);
		fclose(fp);
		
		break;
		
	case SAVE_TREE:
		for (int i = 0; i < MAX_TREE; i++)
		{
			g_TreeSaveData[i] = tree[i];
		}
		
		// セーブデータのチェックサムを求める
		adr = (char*)&g_TreeSaveData;	// デーブデータの先頭アドレスをadrに入れる
		check_sum = 0;
		g_checkSum = 0;
		for (int i = 0; i < sizeof(g_TreeSaveData); i++)
		{
			check_sum += adr[i];
		}
		g_checkSum = check_sum;
		
		// ファイルに出力
		fp = fopen("data/BIN/tree.bin", "wb");
		
		fwrite(&g_TreeSaveData, 1, sizeof(g_TreeSaveData), fp);
		fwrite(&g_checkSum, 1, sizeof(g_checkSum), fp);
		fclose(fp);
		
		break;
	}
}

void LoadData(int load_target)
{
	FILE* fp;
	char* adr;
	int check_sum;
	INSTANCE* grass = getGrassInstance();
	INSTANCE* tree = getTreeInstance();
	
	switch (load_target)
	{
	case SAVE_GRASS:
		// ファイルから読み込み
		fp = fopen("data/BIN/grass.bin", "rb");

		fread(&g_GrassSaveData, 1, sizeof(g_GrassSaveData), fp);
		fread(&g_checkSum, 1, sizeof(g_checkSum), fp);
		fclose(fp);

		// チェックサムを求める
		adr = (char*)&g_GrassSaveData;	// デーブデータの先頭アドレスをadrに入れる
		check_sum = 0;
		for (int i = 0; i < sizeof(g_GrassSaveData); i++)
		{
			check_sum += adr[i];
		}

		// チェックサムが一致しているか確認
		if (check_sum == g_checkSum)
		{
			// 一致していたらセーブデータを読み込む
			for (int i = 0; i < MAX_GRASS; i++)
			{
				grass[i] = g_GrassSaveData[i];
			}
		}
		else
		{
			MessageBox(NULL, "セーブデータが破損しています", "エラー", MB_OK);
		}
		break;

	case SAVE_TREE:
		// ファイルから読み込み
		fp = fopen("data/BIN/tree.bin", "rb");

		fread(&g_TreeSaveData, 1, sizeof(g_TreeSaveData), fp);
		fread(&g_checkSum, 1, sizeof(g_checkSum), fp);
		fclose(fp);
		
		// チェックサムを求める
		adr = (char*)&g_TreeSaveData;	// デーブデータの先頭アドレスをadrに入れる
		check_sum = 0;
		for (int i = 0; i < sizeof(g_TreeSaveData); i++)
		{
			check_sum += adr[i];
		}
		
		// チェックサムが一致しているか確認
		if (check_sum == g_checkSum)
		{
			// 一致していたらセーブデータを読み込む
			for (int i = 0; i < MAX_TREE; i++)
			{
				tree[i] = g_TreeSaveData[i];
			}
		}
		else
		{
			MessageBox(NULL, "セーブデータが破損しています", "エラー", MB_OK);
		}
		break;
	}
}
