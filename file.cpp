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

		// �Z�[�u�f�[�^�̃`�F�b�N�T�������߂�
		adr = (char*)&g_GrassSaveData;	// �f�[�u�f�[�^�̐擪�A�h���X��adr�ɓ����
		check_sum = 0;
		g_checkSum = 0;
		for (int i = 0; i < sizeof(g_GrassSaveData); i++)
		{
			check_sum += adr[i];
		}
		g_checkSum = check_sum;

		// �t�@�C���ɏo��
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
		
		// �Z�[�u�f�[�^�̃`�F�b�N�T�������߂�
		adr = (char*)&g_TreeSaveData;	// �f�[�u�f�[�^�̐擪�A�h���X��adr�ɓ����
		check_sum = 0;
		g_checkSum = 0;
		for (int i = 0; i < sizeof(g_TreeSaveData); i++)
		{
			check_sum += adr[i];
		}
		g_checkSum = check_sum;
		
		// �t�@�C���ɏo��
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
		// �t�@�C������ǂݍ���
		fp = fopen("data/BIN/grass.bin", "rb");

		fread(&g_GrassSaveData, 1, sizeof(g_GrassSaveData), fp);
		fread(&g_checkSum, 1, sizeof(g_checkSum), fp);
		fclose(fp);

		// �`�F�b�N�T�������߂�
		adr = (char*)&g_GrassSaveData;	// �f�[�u�f�[�^�̐擪�A�h���X��adr�ɓ����
		check_sum = 0;
		for (int i = 0; i < sizeof(g_GrassSaveData); i++)
		{
			check_sum += adr[i];
		}

		// �`�F�b�N�T������v���Ă��邩�m�F
		if (check_sum == g_checkSum)
		{
			// ��v���Ă�����Z�[�u�f�[�^��ǂݍ���
			for (int i = 0; i < MAX_GRASS; i++)
			{
				grass[i] = g_GrassSaveData[i];
			}
		}
		else
		{
			MessageBox(NULL, "�Z�[�u�f�[�^���j�����Ă��܂�", "�G���[", MB_OK);
		}
		break;

	case SAVE_TREE:
		// �t�@�C������ǂݍ���
		fp = fopen("data/BIN/tree.bin", "rb");

		fread(&g_TreeSaveData, 1, sizeof(g_TreeSaveData), fp);
		fread(&g_checkSum, 1, sizeof(g_checkSum), fp);
		fclose(fp);
		
		// �`�F�b�N�T�������߂�
		adr = (char*)&g_TreeSaveData;	// �f�[�u�f�[�^�̐擪�A�h���X��adr�ɓ����
		check_sum = 0;
		for (int i = 0; i < sizeof(g_TreeSaveData); i++)
		{
			check_sum += adr[i];
		}
		
		// �`�F�b�N�T������v���Ă��邩�m�F
		if (check_sum == g_checkSum)
		{
			// ��v���Ă�����Z�[�u�f�[�^��ǂݍ���
			for (int i = 0; i < MAX_TREE; i++)
			{
				tree[i] = g_TreeSaveData[i];
			}
		}
		else
		{
			MessageBox(NULL, "�Z�[�u�f�[�^���j�����Ă��܂�", "�G���[", MB_OK);
		}
		break;
	}
}
