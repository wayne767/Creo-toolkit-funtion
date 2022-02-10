// catch_points_R.cpp : 定義 DLL 的初始化常式。
//
#include <stdio.h>
#include "stdafx.h"
#include "catch_points_R.h"
#include "math.h"
#include <stdlib.h>
#include "All_Header_File.h"
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#define PI 3.14159265358979323846

ProError catch_point();
//
//TODO: 如果這個 DLL 是動態地對 MFC DLL 連結，
//		那麼從這個 DLL 匯出的任何會呼叫
//		MFC 內部的函式，都必須在函式一開頭加上 AFX_MANAGE_STATE
//		巨集。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此處為正常函式主體
//		}
//
//		這個巨集一定要出現在每一個
//		函式中，才能夠呼叫 MFC 的內部。這意味著
//		它必須是函式內的第一個陳述式
//		，甚至必須在任何物件變數宣告前面
//		，因為它們的建構函式可能會產生對 MFC
//		DLL 內部的呼叫。
//
//		請參閱 MFC 技術提示 33 和 58 中的
//		詳細資料。
//

// Ccatch_points_RApp

BEGIN_MESSAGE_MAP(Ccatch_points_RApp, CWinApp)
END_MESSAGE_MAP()

// Ccatch_points_RApp 建構

Ccatch_points_RApp::Ccatch_points_RApp()
{
	// TODO: 在此加入建構程式碼，
	// 將所有重要的初始設定加入 InitInstance 中
}

// 僅有的一個 Ccatch_points_RApp 物件

Ccatch_points_RApp theApp;

// Ccatch_points_RApp 初始設定

BOOL Ccatch_points_RApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

static uiCmdAccessState AccessDefault(uiCmdAccessMode access_mode)
{
	return (ACCESS_AVAILABLE);
}
extern "C" int user_initialize()
{
	ProError status;
	ProFileName MsgFile;
	uiCmdCmdId cmd_id1;
	ProStringToWstring(MsgFile, "Message1.txt");
	status = ProMenubarMenuAdd("CHECK", "CHECK", "Utilities", PRO_B_TRUE, MsgFile);
	status = ProCmdActionAdd("Test1", (uiCmdCmdActFn)catch_point, uiCmdPrioDefault, AccessDefault, PRO_B_TRUE, PRO_B_TRUE, &cmd_id1);
	status = ProMenubarmenuPushbuttonAdd("CHECK", "catch_point", "catch_point", "Active catch_point menu", NULL, PRO_B_TRUE, cmd_id1, MsgFile);

	return status;
}
extern "C" void user_terminate()
{
	//end
}

typedef struct asmCompData
{
	ProAsmcomppath comp_path;
	ProSolid prt_handle;
}AsmCompData;

typedef struct //一組三個點
{
	Pro3dPnt catch_pnt1;
	Pro3dPnt catch_pnt2;
	Pro3dPnt mid_point;
	ProVector result_vector;
	Pro3dPnt type;
}CatchPointStruct;
typedef CArray<CatchPointStruct, CatchPointStruct> Point_matrix;

typedef struct //放置所有面的資料
{
	int surface_id;
	ProSurface surface_handle;
}s_info;
typedef CArray<s_info, s_info> s_info_matrix;

typedef struct //放置需要的面的資料
{
	int surface_id;
}s_feasible;
typedef CArray<s_feasible, s_feasible> s_feasible_matrix;

typedef struct //放置不要的面的資料
{
	int surface_id;
}s_futile;
typedef CArray<s_futile, s_futile> s_futile_matrix;

typedef struct //放置中心與所有可用面的距離的資料
{
	double distance;
}DisBtwnPntAndSurf;
typedef CArray<DisBtwnPntAndSurf, DisBtwnPntAndSurf> Dist_matrix;
typedef CArray<int, int> surf_id_array;
typedef CArray<int, int> edge_id_array;
typedef CArray<double,double>  Detection_direction;
ProError SurfaceVisitActionFn(ProSurface surface, ProError err, ProAppData app_data);
ProError SolidSurfacesGet(ProSolid solid, ProSurface **surfs);
ProError AsmCompVisitAct(ProAsmcomppath *p_path, ProSolid handle, ProBoolean down, ProAppData app_data);
ProError getAsmFeature(ProMdl asm_mdl, ProFeature** appdata);
ProError getAsmPrt(ProMdl asm_mdl, AsmCompData** appdata);
ProError FeatureVisitActionFn(ProFeature *feature, ProError err, ProAppData app_data);
ProError FeatureFilterActionFn(ProFeature *feature, ProAppData app_data);
s_info surface_info;
s_info_matrix sorting_surf;
s_futile surface_futile, surface_futile_final;
s_futile_matrix useless_surf, useless_surf_final; //useless surfaces structure
s_feasible surface_feasible, surface_feasible_final;
s_feasible_matrix usable_surf, usable_surf_final; //useable surfaces structureProError CreatFeatPnt(ProMdl model,Pro3dPnt pnt,int *pnt_Id);
void Move_dataOutput(double a, double b, double c, int d);
void Move_dataOutput1(double a, double b, double c);
void Move_dataOutput4(double a, double b);
void Move_dataOutput3(int a);
void Move_dataOutput4(double a, double b, double c);
void Move_dataOutput5(double a, double b, double c);
void Move_dataOutput6(double a, double b, double c, int d);
void Move_dataOutput7(double a, double b, double c, int d);
void Move_dataOutput8(double a, double b, double c);
inline void EnableMemLeakCheck()
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}
ProError catch_point()
{
	ProError err;
	ProEdge edge_handle, bottom_edge_handle;
	ProRay p_ray, p_ray_depth, grap_ray1;
	ProMdl asm_mdl;
	ProSurface *SurfsArray, surf_handle, adjacent_srf[2], interfere_srf_handle, temp_surf, bottom_adjacent_srf[2], adj_surf_handle;
	ProSelection /**p_sel_arr,*/ *p_sel_arr1;
	ProGeomitemdata *s_type_use, *adjacent_srf_data, *bottom_srf_type;
	ProModelitem p_mdl_item;
	ProUvParam uv_pnt_1, curvature_uv;
	ProVector  uv_normal_1, mid_surf_pnt, chosen_uvpnt_F, interference, interference_uv, interference_nor, interference_xyz, edge_normal_direction, rect_norm, edge_point, standard_point, srf_norm, srf_temp, srf_temp_norm, srf_final_normal,interference_direction;
	ProMassProperty solid_property;
	ProFeature *Feature_test;
	ProUvStatus p_result;
	AsmCompData *asm_prt_data;
	s_info surface_info;
	s_info_matrix sorting_surf, sorting_surf_axis;
	s_futile_matrix useless_surf; //useless surfaces structure
	s_feasible surface_feasible;
	s_feasible_matrix usable_surf; //useable surfaces structure
	ProName coordinate;
	/*ProSrftype srf_type;*/
	surf_id_array  surf_id, surf_id_con, surf_id_final, surf_id_final_temp, surf_id_temp, surf_id_con_final, con_temp;
	edge_id_array edge_id_futile;
	Detection_direction Vn;
	ProStringToWstring(coordinate, "ASM_DEF_CSYS");
	int SizeofSurfs, index_contour, index_edge, index_adj, p_cnt, part_num, final_surf_temp = 0, con_check = 0, con_check1 = 0, con_check2 = 0, con_check_minus = 0, con_count = 0, con_temp_count = 1, con_temp_check = 0, adjacent_srf_id, sorting_surf_id, grap_pnt1, bottom_srf = 0, bottom_index_contour, bottom_index_edge;
	int valve = 50, futile_count = 0, feasible_count = 0, feasible_temp_count = 0, surf_count = 0, final_surf = 0, test_count = 0, catch_count = 0, check = 0, check_id = 0, check_id1 = 0, depth_check = 0, Output_check = 0, Output_check1 = 0, Output_check2 = 0, futile_count_check = 0, temp_check = 0, final_check = 0, g_depth_check1 = 0, first_check1 = 0, first_check = 0, futile_count_temp = 0;
	double u_max, v_max, u_mid, v_mid, d_u_1, d_v_1, rad = 0.01, uv_normal_x, uv_normal_y, uv_normal_z, uv_normal_x1, uv_normal_y1, uv_normal_z1, depth, g_depth1, g_depth_check = 0;
	double X, Y, Z, a_area, b_area, surf_area, xx, yy, couut = 0, angle, X_norm, Y_norm, Z_norm, standard_norm[3], area, area_sum = 0, check_direction_vector_temp[3], check_direction_vector[3];
	double edge_para, srf_height, edge_height, distance, distance_check, distance_range = 1000,w=0;
	int  bottom_adjacent_srf_1, bottom_adjacent_srf_2, adj_srf_id_stand, break_check = 0,Jaw_width_inspection=0, problem_points_count=0, Jaw_width_inspection1=0,point_count=0, cover=0;
	//int uv_normal_xf, uv_normal_yf, uv_normal_zf;
	bool interference_check = true;
	double Rotation_matrix_column1[3], Rotation_matrix_column2[3], Rotation_matrix_column3[3],catch_point_a_dis=0,catch_point_b_dis=0,V_direction_X,V_direction_Y,V_direction_Z,V_direction_Z_f,V_direction_X_f,V_direction_Y_f,temp_X,temp_Y,temp_Z;
	///////////////////////////////////////程式碼開始//////////////////////////////////////////////////
	char *savePath =  "E:\\賴以衛\\研究用\\CODE\\新仁智\\Toolkit_30度檢查最終版\\OutPutData.txt";
	char *savePath1 = "E:\\賴以衛\\研究用\\CODE\\新仁智\\Toolkit_30度檢查最終版\\OutPutData(0).txt";
	char *savePath5 = "E:\\賴以衛\\研究用\\CODE\\新仁智\\Toolkit_30度檢查最終版\\OutPutData(1).txt";
	char *savePath2 = "E:\\賴以衛\\研究用\\CODE\\新仁智\\Toolkit_30度檢查最終版\\Detection_direction.txt";
	char *savePath3 = "E:\\賴以衛\\研究用\\CODE\\新仁智\\Toolkit_30度檢查最終版\\COD.txt";
	char *savePath6 = "E:\\賴以衛\\研究用\\CODE\\新仁智\\Toolkit_30度檢查最終版\\time_robot.txt";
	if (remove(savePath) == 0)
	{
	}
	if (remove(savePath1) == 0)
	{
	}
	if (remove(savePath2) == 0)
	{
	}	
	if (remove(savePath3) == 0)
	{
	}
	if (remove(savePath5) == 0)
	{
	}
	if (remove(savePath6) == 0)
	{
	}
	else
	{
	}
	err = ProMdlCurrentGet(&asm_mdl); //取得組件的handle(控制權)
	err = getAsmFeature(asm_mdl, &Feature_test);//Get feature parts blocks 取得組件下的feature
	err = getAsmPrt(asm_mdl, &asm_prt_data); //取得組件下的PART
	err = ProArraySizeGet(asm_prt_data, &part_num);//Get array size   組件中看有幾個零件
	time_t begin, end;
	begin = clock();
	for (int number = 0; number < part_num; number++)
	{
		surf_id.RemoveAll();
		surf_id_con.RemoveAll();
		surf_id_final.RemoveAll();
		surf_id_temp.RemoveAll();
		surf_id_final_temp.RemoveAll();
		edge_id_futile.RemoveAll();
		point_count = 0;
		feasible_count = 0;
		surf_count = 0;
		final_surf = 0;
		futile_count = 0;
		feasible_temp_count = 0;
		temp_check = 0;
		area_sum = 0;
		err = SolidSurfacesGet(asm_prt_data[number].prt_handle, &SurfsArray);//組件中第一個prt的所有面handle放入
		err = ProArraySizeGet(SurfsArray, &SizeofSurfs);  //面的數量
		//////////////////
		//for (int i = 0; i < SizeofSurfs; i++)
		//{
		//	err=ProSurfaceTypeGet(SurfsArray[i],&srf_type);
		//	if (srf_type==PRO_SRF_CYL)
		//	{
		//		ProSurfaceIdGet(SurfsArray[i], &surface_info.surface_id);
		//	}
		//}													
		err = ProSolidMassPropertyGet((ProSolid)asm_prt_data[number].prt_handle, NULL, &solid_property);
		Move_dataOutput1(solid_property.center_of_gravity[0], solid_property.center_of_gravity[1], solid_property.center_of_gravity[2]);
		for (int s = 1; s<SizeofSurfs; s++) //sorting out by squence  
		{
			for (int j = 0; j<SizeofSurfs - 1; j++)
			{
				err = ProSurfaceAreaEval(SurfsArray[j], &a_area);
				err = ProSurfaceAreaEval(SurfsArray[j + 1], &b_area);
				if (a_area<b_area)
				{
					temp_surf = SurfsArray[j];
					SurfsArray[j] = SurfsArray[j + 1];
					SurfsArray[j + 1] = temp_surf;
				}
			}
		}
		for (int i = 0; i < SizeofSurfs; i++) //計算總表面積
		{
			ProSurfaceAreaEval(SurfsArray[i], &area);
			area_sum = area_sum + area;
		}
		double standard_area = area_sum / SizeofSurfs; //calculates standard area
		for (int i = 0; i < SizeofSurfs; i++)
		{
			ProSurfaceIdGet(SurfsArray[i], &surface_info.surface_id);  //將面id放入surface_info
			surface_info.surface_handle = SurfsArray[i];
			sorting_surf.Add(surface_info);
		}
		for (int i = 0; i < SizeofSurfs; i++)
		{
			err = ProSurfaceInit((ProMdl)asm_prt_data[number].prt_handle, sorting_surf[i].surface_id, &surf_handle);
			err = ProSurfaceDataGet(surf_handle, &s_type_use);
			sorting_surf_id = sorting_surf[i].surface_id;
			err = ProArraySizeGet(s_type_use->data.p_surface_data->contour_arr, &index_contour);
			//if (sorting_surf_id == 53 || sorting_surf_id == 48)
			//{
			//	int asdsad = 0;
			//}
			if (index_contour >= 2) //該面有2個以上的迴圈
			{
				/*surf_id.Add(sorting_surf[i].surface_id);*/
				surf_id_temp.Add(sorting_surf[i].surface_id);
				//feasible_count++; //可用
				feasible_temp_count++;
				//o_u_mid = (s_type_use->data.p_surface_data->uv_max[0] - s_type_use->data.p_surface_data->uv_min[0]) / 2;
				//o_v_mid = (s_type_use->data.p_surface_data->uv_max[1] - s_type_use->data.p_surface_data->uv_min[1]) / 2;
				//o_uv[0] = s_type_use->data.p_surface_data->uv_min[0] + o_u_mid;
				//o_uv[1] = s_type_use->data.p_surface_data->uv_min[1] + o_v_mid; //subordinate adjacent surface middle U-V pnt								
				//err = ProSurfaceXyzdataEval(surf_handle, o_uv, NULL, NULL, NULL, srf_norm);//uv座標轉xyz座標
				for (int j = 0; j < index_contour; j++)
				{
					if (s_type_use->data.p_surface_data->contour_arr[j].trav == PRO_CONTOUR_TRAV_INTERNAL) //判斷為內迴圈
					{
						err = ProArraySizeGet(s_type_use->data.p_surface_data->contour_arr[j].edge_id_arr, &index_edge); //邊的數量
						for (int k = 0; k < index_edge; k++)
						{
							err = ProEdgeInit(asm_prt_data[number].prt_handle, s_type_use->data.p_surface_data->contour_arr[j].edge_id_arr[k], &edge_handle);//找出邊的handle
							err = ProEdgeNeighborsGet(edge_handle, NULL, NULL, &adjacent_srf[0], &adjacent_srf[1]);//藉由邊的handle求出2個連接面
							for (int l = 0; l < 2; l++)
							{
								err = ProSurfaceIdGet(adjacent_srf[l], &adjacent_srf_id);
								if (adjacent_srf_id != sorting_surf_id) //不等於原先的面
								{

									index_adj = l;
									distance_range = 1000;
									err = ProSurfaceInit((ProMdl)asm_prt_data[number].prt_handle, adjacent_srf_id, &adj_surf_handle);
									adj_srf_id_stand = adjacent_srf_id;
									err = ProSurfaceDataGet(adjacent_srf[index_adj], &adjacent_srf_data);
									u_mid = (adjacent_srf_data->data.p_surface_data->uv_max[0] - adjacent_srf_data->data.p_surface_data->uv_min[0]) / 2;
									v_mid = (adjacent_srf_data->data.p_surface_data->uv_max[1] - adjacent_srf_data->data.p_surface_data->uv_min[1]) / 2;
									curvature_uv[0] = adjacent_srf_data->data.p_surface_data->uv_min[0] + u_mid;
									curvature_uv[1] = adjacent_srf_data->data.p_surface_data->uv_min[1] + v_mid; //subordinate adjacent surface middle U-V pnt								
									err = ProSurfaceXyzdataEval(adjacent_srf[index_adj], curvature_uv, mid_surf_pnt, NULL, NULL, rect_norm);//uv座標轉xyz座標
																																			//err = ProSurfaceParamEval(asm_prt_data[number].prt_handle,surf_handle,mid_surf_pnt,o_uv);
																																			//err = ProSurfaceXyzdataEval(surf_handle, o_uv, NULL, NULL, NULL, srf_norm);//uv座標轉xyz座標
									ProEdgeParamEval(edge_handle, mid_surf_pnt, &edge_para);
									ProEdgeXyzdataEval(edge_handle, edge_para, edge_point, NULL, NULL, edge_normal_direction);
									distance = sqrt(pow((mid_surf_pnt[0] - edge_point[0]), 2) + pow((mid_surf_pnt[1] - edge_point[1]), 2) + pow((mid_surf_pnt[2] - edge_point[2]), 2));
									u_max = s_type_use->data.p_surface_data->uv_max[0] - s_type_use->data.p_surface_data->uv_min[0];//U最大邊
									v_max = s_type_use->data.p_surface_data->uv_max[1] - s_type_use->data.p_surface_data->uv_min[1];//V最大邊
									d_u_1 = u_max / 10;
									d_v_1 = v_max / 10;
									xx = 0.5;
									yy = 0.5;
									for (double uu = 0; uu < 10; uu = uu + xx)
									{
										for (double vv = 0; vv < 10; vv = vv + yy)
										{
											uv_pnt_1[0] = s_type_use->data.p_surface_data->uv_max[0] - (uu*d_u_1);
											uv_pnt_1[1] = s_type_use->data.p_surface_data->uv_max[1] - (vv*d_v_1);
											err = ProSurfaceUvpntVerify(asm_prt_data[number].prt_handle, surf_handle, uv_pnt_1, &p_result);
											if (p_result == PRO_UV_INSIDE)
											{
												err = ProSurfaceXyzdataEval(surf_handle, uv_pnt_1, srf_norm, NULL, NULL, srf_temp_norm);//uv座標轉xyz座標
												distance_check = sqrt(pow((srf_norm[0] - edge_point[0]), 2) + pow((srf_norm[1] - edge_point[1]), 2) + pow((srf_norm[2] - edge_point[2]), 2));
												if (distance_check<distance_range)//找出B點
												{
													distance_range = distance_check;
													srf_temp[0] = srf_temp_norm[0];
													srf_temp[1] = srf_temp_norm[1];
													srf_temp[2] = srf_temp_norm[2];
												}
											}
										}
									}

									srf_final_normal[0] = srf_temp[0] / sqrt(pow(srf_temp[0], 2) + pow(srf_temp[1], 2) + pow(srf_temp[2], 2));
									srf_final_normal[1] = srf_temp[1] / sqrt(pow(srf_temp[0], 2) + pow(srf_temp[1], 2) + pow(srf_temp[2], 2));
									srf_final_normal[2] = srf_temp[2] / sqrt(pow(srf_temp[0], 2) + pow(srf_temp[1], 2) + pow(srf_temp[2], 2));


									standard_point[0] = mid_surf_pnt[0] + srf_final_normal[0] * (distance + 10);
									standard_point[1] = mid_surf_pnt[1] + srf_final_normal[1] * (distance + 10);
									standard_point[2] = mid_surf_pnt[2] + srf_final_normal[2] * (distance + 10);
									/*A*/	edge_height = sqrt(pow((standard_point[0] - edge_point[0]), 2) + pow((standard_point[1] - edge_point[1]), 2) + pow((standard_point[2] - edge_point[2]), 2));
									/*B*/	srf_height = sqrt(pow((standard_point[0] - mid_surf_pnt[0]), 2) + pow((standard_point[1] - mid_surf_pnt[1]), 2) + pow((standard_point[2] - mid_surf_pnt[2]), 2));
									if (edge_height < srf_height)
									{
										surf_id_con.Add(adjacent_srf_id);
										futile_count++;
										bottom_srf++;
									}
									/*if (edge_height > srf_height)
									{
										surf_id.Add(sorting_surf_id);
										feasible_count++;
									}*/
									/*if (edge_height > srf_height)
									{
										surf_id_con.Add(sorting_surf[i].surface_id);
										futile_count++;
										interference_check = false;
									}*/

								}
							}
						}

						if (bottom_srf > 0)
						{
							err = ProSurfaceDataGet(adj_surf_handle, &bottom_srf_type);
							err = ProArraySizeGet(bottom_srf_type->data.p_surface_data->contour_arr, &bottom_index_contour);
							for (size_t f = 0; f < bottom_index_contour; f++)
							{
								if (bottom_srf_type->data.p_surface_data->contour_arr[f].trav == PRO_CONTOUR_TRAV_EXTERNAL)
								{
									err = ProArraySizeGet(bottom_srf_type->data.p_surface_data->contour_arr[f].edge_id_arr, &bottom_index_edge); //邊的數量
									for (size_t g = 0; g < bottom_index_edge; g++)
									{
										err = ProEdgeInit(asm_prt_data[number].prt_handle, bottom_srf_type->data.p_surface_data->contour_arr[f].edge_id_arr[g], &bottom_edge_handle);//找出邊的handle
										err = ProEdgeNeighborsGet(bottom_edge_handle, NULL, NULL, &bottom_adjacent_srf[0], &bottom_adjacent_srf[1]);//藉由邊的handle求出2個連接面
										err = ProSurfaceIdGet(bottom_adjacent_srf[0], &bottom_adjacent_srf_1);
										err = ProSurfaceIdGet(bottom_adjacent_srf[1], &bottom_adjacent_srf_2);
										if (bottom_adjacent_srf_1 == adj_srf_id_stand && bottom_adjacent_srf_2 != sorting_surf_id)
										{
											surf_id_con.Add(bottom_adjacent_srf_2);
											futile_count++;
											break_check++;
										}
										else if (bottom_adjacent_srf_1 == adj_srf_id_stand && bottom_adjacent_srf_2 == sorting_surf_id)
										{
											surf_id_con.Add(bottom_adjacent_srf_1);
											futile_count++;
											break_check++;
										}
										else if (bottom_adjacent_srf_2 == adj_srf_id_stand && bottom_adjacent_srf_1 != sorting_surf_id)
										{
											surf_id_con.Add(bottom_adjacent_srf_1);
											futile_count++;
											break_check++;
										}
										else if (bottom_adjacent_srf_2 == adj_srf_id_stand && bottom_adjacent_srf_1 == sorting_surf_id)
										{
											surf_id_con.Add(bottom_adjacent_srf_2);
											futile_count++;
											break_check++;
										}
									}
								}
								if (break_check > 0)
								{
									break;
								}
							}
						}
						break_check = 0;
						bottom_srf = 0;
					}
				}
				/*if (interference_check == true)
				{
					surf_id.Add(sorting_surf_id);
					feasible_count++;
				}
				interference_check = true;*/
			}
			else if (index_contour == 1)
			{
				surf_id.Add(sorting_surf_id);
				feasible_count++;
			}

		}
		//////////////////////////////////////////////篩選面id，當id在surf_id_con中，將其排除////////////////////////////////////////////////////
		for (int i = 0; i < feasible_count; i++)
		{
			for (int j = 0; j < futile_count; j++)
			{
				test_count = 0;
				int ggg = surf_id[i];
				int tttt = surf_id_con[j];
				if (surf_id[i] == surf_id_con[j])
				{
					test_count = 1;
					break;
				}
			}
			if (test_count != 1)
			{
				surf_id_final.Add(surf_id[i]);
				final_surf++;
			}
		}
		////////////////////////將有兩迴圈的面ID加入surf_id_final_temp//////////////////
		for (int i = 0; i < feasible_temp_count; i++)
		{
			for (int j = 0; j < final_surf; j++)
			{
				int aaaaaa = surf_id_temp[i];
				int aaaaab = surf_id_final[j];

				if (surf_id_temp[i] == surf_id_final[j])
				{
					break;
				}
				else
				{
					temp_check++;
				}
			}
			if (temp_check == final_surf)
			{
				surf_id_final_temp.Add(surf_id_temp[i]);
				final_surf_temp++;
			}
			temp_check = 0;
		}
		///////////////////////////匯入最終資料//////////////////////
		for (int i = 0; i < final_surf; i++)
		{
			int ttt = surf_id_final[i];
			surface_feasible.surface_id = surf_id_final[i];
			usable_surf.Add(surface_feasible);
		}
		if (final_surf_temp>0)
		{
			for (int i = 0; i < final_surf_temp; i++)
			{
				int yyy = surf_id_final_temp[i];
				surface_feasible.surface_id = surf_id_final_temp[i];
				usable_surf.Add(surface_feasible);
				final_surf++;
				con_check_minus++;
			}
		}
		//////////////////////////////////////////////篩選無用面重複id////////////////////////////////////////////////////
		for (size_t i = 0; i < futile_count; i++)
		{
			for (size_t j = 0; j < feasible_temp_count; j++) //剔除最終面
			{
				int cc = surf_id_con[i];
				int ccccc = surf_id_temp[j];
				if (surf_id_con[i] == surf_id_temp[j])
				{
					con_check++;
				}
			}
			for (size_t k = 0; k < (final_surf - con_check_minus); k++) //剃除二次挑選面
			{
				int ccccccccc = surf_id_final[k];
				if (surf_id_con[i] == surf_id_final[k]) //不能有可用面
				{
					con_check1++;
				}
			}
			for (int o = 0; o < con_count; o++)
			{
				if (surf_id_con[i] == surf_id_con_final[o])
				{
					final_check++;
				}
			}
			if (con_check == 0 && con_check1 == 0 && final_check == 0)
			{
				surf_id_con_final.Add(surf_id_con[i]); //最終條件面
				con_count++;
			}
			con_check = 0;
			con_check1 = 0;
			final_check = 0;
		}
	/*	for (int i = 0; i < con_count; i++)
		{
			Move_dataOutput3(surf_id_con_final[i]);
		}*/
		/*Move_dataOutput3(111111);
		for (int i = 0; i < final_surf; i++)
		{
			Move_dataOutput3(usable_surf[i].surface_id);
		}*/
		surf_id.RemoveAll();
		surf_id_final.RemoveAll();
		surf_id_temp.RemoveAll();
		surf_id_final_temp.RemoveAll();
		///////////////////將篩選面進行可用點分析//////////////////////////////////////
		for (int u = 0; u < final_surf; u++)
		{
			int aaaaaaaa=usable_surf[u].surface_id;
			if (usable_surf[u].surface_id==48)
			{
				int aaaa;
				aaaa=00010;
			}
			err = ProSurfaceInit((ProMdl)asm_prt_data[number].prt_handle, usable_surf[u].surface_id, &surf_handle); //將可用面進行更詳細之佈點
			err = ProSurfaceDataGet(surf_handle, &s_type_use);
			err = ProSurfaceAreaEval(surf_handle, &surf_area);
			/*Move_dataOutput2(surf_area, usable_surf[u].surface_id);*/
			u_max = s_type_use->data.p_surface_data->uv_max[0] - s_type_use->data.p_surface_data->uv_min[0];//U最大邊
			v_max = s_type_use->data.p_surface_data->uv_max[1] - s_type_use->data.p_surface_data->uv_min[1];//V最大邊
			d_u_1 = u_max / valve;
			d_v_1 = v_max / valve;
			//////////////////////佈點/////////////////////
			if (surf_area<standard_area)
			{
				xx = 2, yy = 2;
			}
			else
			{
				xx = 1, yy = 1;
			}
			for (double uu = 0.1; uu < valve; uu = uu + xx)
			{
				for (double vv = 0.1; vv < valve; vv = vv + yy)
				{
					Jaw_width_inspection = 0;
					Jaw_width_inspection1 = 0;
					futile_count_check = 0;
					uv_pnt_1[0] = s_type_use->data.p_surface_data->uv_max[0] - (uu*d_u_1);
					uv_pnt_1[1] = s_type_use->data.p_surface_data->uv_max[1] - (vv*d_v_1);
					err = ProSurfaceUvpntVerify(asm_prt_data[number].prt_handle, surf_handle, uv_pnt_1, &p_result);
					if (p_result == PRO_UV_INSIDE)
					{
						err = ProSurfaceXyzdataEval(surf_handle, uv_pnt_1, chosen_uvpnt_F, NULL, NULL, uv_normal_1);//uv座標轉xyz座標
						uv_normal_x = uv_normal_1[0] / sqrt(pow(uv_normal_1[0], 2) + pow(uv_normal_1[1], 2) + pow(uv_normal_1[2], 2));
						uv_normal_y = uv_normal_1[1] / sqrt(pow(uv_normal_1[0], 2) + pow(uv_normal_1[1], 2) + pow(uv_normal_1[2], 2));
						uv_normal_z = uv_normal_1[2] / sqrt(pow(uv_normal_1[0], 2) + pow(uv_normal_1[1], 2) + pow(uv_normal_1[2], 2));
						
						X_norm = abs(uv_normal_x);
						Y_norm = abs(uv_normal_y);
						Z_norm = abs(uv_normal_z);

						if (X_norm == 1)  //求得與夾取點方向進行外積的軸向
						{
							standard_norm[0] = 0; standard_norm[1] = 1; standard_norm[2] = 0;
						}
						else if (Y_norm == 1)
						{
							standard_norm[0] = 0; standard_norm[1] = 0; standard_norm[2] = 1;
						}
						else if (Z_norm == 1)
						{
							standard_norm[0] = 1; standard_norm[1] = 0; standard_norm[2] = 0;
						}
						else
						{
							if (X_norm<Y_norm&&X_norm<Z_norm)
							{
								standard_norm[0] = 1; standard_norm[1] = 0; standard_norm[2] = 0;
							}
							if (Y_norm<X_norm&&Y_norm<Z_norm)
							{
								standard_norm[0] = 0; standard_norm[1] = 1; standard_norm[2] = 0;
							}
							if (Z_norm<Y_norm&&Z_norm<X_norm)
							{
								standard_norm[0] = 0; standard_norm[1] = 0; standard_norm[2] = 1;
							}
						}
						check_direction_vector_temp[0] = standard_norm[1] * uv_normal_z - standard_norm[2] * uv_normal_y;  //向量V1
						check_direction_vector_temp[1] = standard_norm[2] * uv_normal_x - standard_norm[0] * uv_normal_z;
						check_direction_vector_temp[2] = standard_norm[0] * uv_normal_y - standard_norm[1] * uv_normal_x;
						grap_ray1.start_point[0] = chosen_uvpnt_F[0] + uv_normal_x*1; //夾取點偏移
						grap_ray1.start_point[1] = chosen_uvpnt_F[1] + uv_normal_y*1;
						grap_ray1.start_point[2] = chosen_uvpnt_F[2] + uv_normal_z*1;
						temp_X = chosen_uvpnt_F[0] + uv_normal_x*1; //夾取點偏移
						temp_Y = chosen_uvpnt_F[1] + uv_normal_y*1;
						temp_Z = chosen_uvpnt_F[2] + uv_normal_z*1;
						Move_dataOutput8(chosen_uvpnt_F[0],chosen_uvpnt_F[1],chosen_uvpnt_F[2]);//輸出射線法佈點
						for (double angle_test=0; angle_test < 180; angle_test=angle_test+45)
						{
							w=cos(angle_test*PI/180);
							Rotation_matrix_column1[0] = 1-2*(pow(Y_norm,2)+pow(Z_norm,2)); Rotation_matrix_column1[1] = 2*(X_norm*Y_norm-w*Z_norm); Rotation_matrix_column1[2] = 2*(w*Y_norm+Z_norm*X_norm);
							Rotation_matrix_column2[0] = 2*(X_norm*Y_norm+w*Z_norm); Rotation_matrix_column2[1] = 1-2*(pow(X_norm,2)+pow(Z_norm,2)) ; Rotation_matrix_column2[2] = 2*(Z_norm*Y_norm-w*X_norm);
							Rotation_matrix_column3[0] = 2*(Z_norm*X_norm-w*Y_norm); Rotation_matrix_column3[1] = 2*(Z_norm*Y_norm+w*X_norm); Rotation_matrix_column3[2] = 1-2*(pow(Y_norm,2)+pow(X_norm,2)) ;
							check_direction_vector[0] = Rotation_matrix_column1[0] * check_direction_vector_temp[0] + Rotation_matrix_column1[1] * check_direction_vector_temp[1] + Rotation_matrix_column1[2] * check_direction_vector_temp[2];
							check_direction_vector[1] = Rotation_matrix_column2[0] * check_direction_vector_temp[0] + Rotation_matrix_column2[1] * check_direction_vector_temp[1] + Rotation_matrix_column2[2] * check_direction_vector_temp[2];
							check_direction_vector[2] = Rotation_matrix_column3[0] * check_direction_vector_temp[0] + Rotation_matrix_column3[1] * check_direction_vector_temp[1] + Rotation_matrix_column3[2] * check_direction_vector_temp[2];
							grap_ray1.dir_vector[0] = check_direction_vector[0];
							grap_ray1.dir_vector[1] = check_direction_vector[1];
							grap_ray1.dir_vector[2] = check_direction_vector[2];
							err = ProSolidRayIntersectionCompute(asm_prt_data[number].prt_handle, rad, &grap_ray1, &p_sel_arr1, &grap_pnt1);
							//for (int oo = 0; oo < grap_pnt1; oo++)
							//{
							//	err = ProSelectionPoint3dGet(p_sel_arr[oo], interference);//干涉點座標
							//}	
							if (grap_pnt1>0)//代表下爪方向有干涉
							{
								for (int x = 0; x < grap_pnt1 - 1; x++) //檢查夾取點是否鄰近凸特徵
								{
									err = ProSelectionDepthGet(p_sel_arr1[x], &g_depth1);
									if (abs(g_depth1)<10)
									{
										Jaw_width_inspection++;
										problem_points_count = 0;
										Vn.RemoveAll();
									}
								}
								if (Jaw_width_inspection>0)
								{
									break;
								}
								
								V_direction_X_f = check_direction_vector[0] / sqrt(pow(check_direction_vector[0], 2) + pow(check_direction_vector[1], 2) + pow(check_direction_vector[2], 2));
								V_direction_Y_f = check_direction_vector[1] / sqrt(pow(check_direction_vector[0], 2) + pow(check_direction_vector[1], 2) + pow(check_direction_vector[2], 2));
								V_direction_Z_f = check_direction_vector[2] / sqrt(pow(check_direction_vector[0], 2) + pow(check_direction_vector[1], 2) + pow(check_direction_vector[2], 2));
								problem_points_count++;    //紀錄產生干涉的檢測數量
								Vn.Add(V_direction_X_f);
								Vn.Add(V_direction_Y_f);
								Vn.Add(V_direction_Z_f);
							}

							
						}
						if (Jaw_width_inspection>0)
						{
							break;
						}
						///////////////////////
						if (Jaw_width_inspection == 0)
						{
							p_ray.start_point[0] = chosen_uvpnt_F[0];  //以夾取點A求得另一側夾取點
							p_ray.start_point[1] = chosen_uvpnt_F[1];
							p_ray.start_point[2] = chosen_uvpnt_F[2];
							p_ray.dir_vector[0] = uv_normal_x;
							p_ray.dir_vector[1] = uv_normal_y;
							p_ray.dir_vector[2] = uv_normal_z;
							err = ProSolidRayIntersectionCompute(asm_prt_data[number].prt_handle, rad, &p_ray, &p_sel_arr1, &p_cnt);
							if (p_cnt >= 2)
							{
								err = ProSelectionPoint3dGet(p_sel_arr1[0], interference);//干涉點座標
								err = ProSelectionModelitemGet(p_sel_arr1[0], &p_mdl_item); //找到面的資料
								err = ProSurfaceInit(p_mdl_item.owner, p_mdl_item.id, &interfere_srf_handle); //拉出干涉面的handle，為求surface type
								for (size_t d = 0; d < con_count; d++)
								{
									if (p_mdl_item.id == surf_id_con_final[d])
									{
										futile_count_check++;
									}
								} //判斷是否產生在排除面上
								if (futile_count_check == 0)
								{
									err = ProSurfaceParamEval(asm_prt_data[number].prt_handle, interfere_srf_handle, interference, interference_uv);//xyz給uv  interference->夾取點B
									err = ProSurfaceXyzdataEval(interfere_srf_handle, interference_uv, interference_xyz, NULL, NULL, interference_nor);//求得夾取點B的法向量
									uv_normal_x1 = interference_nor[0] / sqrt(pow(interference_nor[0], 2) + pow(interference_nor[1], 2) + pow(interference_nor[2], 2));
									uv_normal_y1 = interference_nor[1] / sqrt(pow(interference_nor[0], 2) + pow(interference_nor[1], 2) + pow(interference_nor[2], 2));
									uv_normal_z1 = interference_nor[2] / sqrt(pow(interference_nor[0], 2) + pow(interference_nor[1], 2) + pow(interference_nor[2], 2));
									angle = (180 / PI)*acos((uv_normal_x*uv_normal_x1 + uv_normal_y*uv_normal_y1 + uv_normal_z*uv_normal_z1) / ((sqrt(pow(uv_normal_x1, 2) + pow(uv_normal_y1, 2) + pow(uv_normal_z1, 2)))*(sqrt(pow(uv_normal_x, 2) + pow(uv_normal_y, 2) + pow(uv_normal_z, 2)))));
									if (angle>120 && angle<300)//判斷兩法向量間的夾角
									{
										X = (chosen_uvpnt_F[0] + interference[0]) / 2; //(夾取點A+夾取點B)/2 求得中點
										Y = (chosen_uvpnt_F[1] + interference[1]) / 2;
										Z = (chosen_uvpnt_F[2] + interference[2]) / 2;
										X_norm = abs(uv_normal_x1);     //夾取點B的法向量
										Y_norm = abs(uv_normal_y1);
										Z_norm = abs(uv_normal_z1);
										grap_ray1.start_point[0] = X;
										grap_ray1.start_point[1] = Y;
										grap_ray1.start_point[2] = Z;
										grap_ray1.dir_vector[0] = uv_normal_x1;
										grap_ray1.dir_vector[1] = uv_normal_y1;
										grap_ray1.dir_vector[2] = uv_normal_z1;
										catch_point_a_dis = sqrt(pow((interference[0] - X), 2) + pow((interference[1] - Y), 2) + pow((interference[2] - Z), 2)); //判斷夾取點AB是否被遮蔽
										catch_point_b_dis = sqrt(pow((chosen_uvpnt_F[0] - X), 2) + pow((chosen_uvpnt_F[1] - Y), 2) + pow((chosen_uvpnt_F[2] - Z), 2));
										err = ProSolidRayIntersectionCompute(asm_prt_data[number].prt_handle, rad, &grap_ray1, &p_sel_arr1, &grap_pnt1);
										for (int x = 0; x < grap_pnt1 - 1; x++)
										{
											err = ProSelectionDepthGet(p_sel_arr1[x], &g_depth1);
											if (abs(g_depth1)<30 && abs(g_depth1)>catch_point_a_dis)
											{
												cover++;
											}
											/*if (abs(g_depth1)<30&&abs(g_depth1)!=(catch_point_a_dis+0.5))
											{
											cover++;
											}*/
										}
										if (cover == 0)//當夾取點未被遮蔽
										{
											if (X_norm == 1)
											{
												standard_norm[0] = 0; standard_norm[1] = 1; standard_norm[2] = 0;
											}
											else if (Y_norm == 1)
											{
												standard_norm[0] = 0; standard_norm[1] = 0; standard_norm[2] = 1;
											}
											else if (Z_norm == 1)
											{
												standard_norm[0] = 1; standard_norm[1] = 0; standard_norm[2] = 0;
											}
											else
											{
												if (X_norm<Y_norm&&X_norm<Z_norm)
												{
													standard_norm[0] = 1; standard_norm[1] = 0; standard_norm[2] = 0;
												}
												if (Y_norm<X_norm&&Y_norm<Z_norm)
												{
													standard_norm[0] = 0; standard_norm[1] = 1; standard_norm[2] = 0;
												}
												if (Z_norm<Y_norm&&Z_norm<X_norm)
												{
													standard_norm[0] = 0; standard_norm[1] = 0; standard_norm[2] = 1;
												}
											}
											check_direction_vector_temp[0] = standard_norm[1] * uv_normal_z1 - standard_norm[2] * uv_normal_y1;  //求得夾取點B的檢測向量
											check_direction_vector_temp[1] = standard_norm[2] * uv_normal_x1 - standard_norm[0] * uv_normal_z1;
											check_direction_vector_temp[2] = standard_norm[0] * uv_normal_y1 - standard_norm[1] * uv_normal_x1;
											grap_ray1.start_point[0] = interference[0] + uv_normal_x1 * 1; //中點向外偏移獲得偏移點O
											grap_ray1.start_point[1] = interference[1] + uv_normal_y1 * 1;
											grap_ray1.start_point[2] = interference[2] + uv_normal_z1 * 1;

											for (double angle_test = 0; angle_test < 180; angle_test = angle_test + 30)//夾取點B的下爪方向檢查
											{
												w = cos(angle_test*PI / 180);//讓向量對某個單位向量軸旋轉角度2θ 因此讓w=cosθ
												Rotation_matrix_column1[0] = 1 - 2 * (pow(Y_norm, 2) + pow(Z_norm, 2)); Rotation_matrix_column1[1] = 2 * (X_norm*Y_norm - w*Z_norm); Rotation_matrix_column1[2] = 2 * (w*Y_norm + Z_norm*X_norm);
												Rotation_matrix_column2[0] = 2 * (X_norm*Y_norm + w*Z_norm); Rotation_matrix_column2[1] = 1 - 2 * (pow(X_norm, 2) + pow(Z_norm, 2)); Rotation_matrix_column2[2] = 2 * (Z_norm*Y_norm - w*X_norm);
												Rotation_matrix_column3[0] = 2 * (Z_norm*X_norm - w*Y_norm); Rotation_matrix_column3[1] = 2 * (Z_norm*Y_norm + w*X_norm); Rotation_matrix_column3[2] = 1 - 2 * (pow(Y_norm, 2) + pow(X_norm, 2));
												check_direction_vector[0] = Rotation_matrix_column1[0] * check_direction_vector_temp[0] + Rotation_matrix_column1[1] * check_direction_vector_temp[1] + Rotation_matrix_column1[2] * check_direction_vector_temp[2];
												check_direction_vector[1] = Rotation_matrix_column2[0] * check_direction_vector_temp[0] + Rotation_matrix_column2[1] * check_direction_vector_temp[1] + Rotation_matrix_column2[2] * check_direction_vector_temp[2];
												check_direction_vector[2] = Rotation_matrix_column3[0] * check_direction_vector_temp[0] + Rotation_matrix_column3[1] * check_direction_vector_temp[1] + Rotation_matrix_column3[2] * check_direction_vector_temp[2];
												grap_ray1.dir_vector[0] = check_direction_vector[0];
												grap_ray1.dir_vector[1] = check_direction_vector[1];
												grap_ray1.dir_vector[2] = check_direction_vector[2];
												err = ProSolidRayIntersectionCompute(asm_prt_data[number].prt_handle, rad, &grap_ray1, &p_sel_arr1, &grap_pnt1);

												if (grap_pnt1>0)//檢查夾取點B下爪方向
												{
													for (int x = 0; x < grap_pnt1 - 1; x++) //檢查夾取點是否鄰近凸特徵
													{
														err = ProSelectionDepthGet(p_sel_arr1[x], &g_depth1);
														if (abs(g_depth1)<10)
														{
															problem_points_count = 0;
															Vn.RemoveAll();
															Jaw_width_inspection1++;
														}
													}
													if (Jaw_width_inspection1>0)
													{
														break;
													}
													V_direction_X_f = check_direction_vector[0] / sqrt(pow(check_direction_vector[0], 2) + pow(check_direction_vector[1], 2) + pow(check_direction_vector[2], 2));
													V_direction_Y_f = check_direction_vector[1] / sqrt(pow(check_direction_vector[0], 2) + pow(check_direction_vector[1], 2) + pow(check_direction_vector[2], 2));
													V_direction_Z_f = check_direction_vector[2] / sqrt(pow(check_direction_vector[0], 2) + pow(check_direction_vector[1], 2) + pow(check_direction_vector[2], 2));
													problem_points_count++;    //紀錄產生干涉的檢測數量
													Vn.Add(V_direction_X_f);
													Vn.Add(V_direction_Y_f);
													Vn.Add(V_direction_Z_f);
												}
											}
											if (Jaw_width_inspection1>0)
											{
												break;
											}
											p_ray_depth.start_point[0] = X;
											p_ray_depth.start_point[1] = Y;
											p_ray_depth.start_point[2] = Z;
											p_ray_depth.dir_vector[0] = uv_normal_x;
											p_ray_depth.dir_vector[1] = uv_normal_y;
											p_ray_depth.dir_vector[2] = uv_normal_z;
											err = ProSolidRayIntersectionCompute(asm_prt_data[number].prt_handle, rad, &p_ray_depth, &p_sel_arr1, &p_cnt);

											//for (int y = 0; y < p_cnt - 1; y++) //夾爪寬度
											//{
											//err = ProSelectionDepthGet(p_sel_arr1[y], &depth);
										
											//if (abs(depth)>30 && abs(depth)<35)
											//{
											//depth_check++;
											//}
											//}
											if (depth_check == 0)
											{
												if (problem_points_count>0)
												{
													point_count++;
													Move_dataOutput(chosen_uvpnt_F[0], chosen_uvpnt_F[1], chosen_uvpnt_F[2], point_count);
													Move_dataOutput(interference[0], interference[1], interference[2], point_count);
													Move_dataOutput(X, Y, Z, point_count);
													//////////////////////////////////////////
													double dx,dy,dz,dx1,dy1,dz1,dxx,dyy,dzz,distance;  
													dx   =   chosen_uvpnt_F[0]; 
													dy   =   chosen_uvpnt_F[1]; 
													dz   =   chosen_uvpnt_F[2]; 
													dx1  =   interference[0]; 
													dy1  =   interference[1]; 
													dz1  =   interference[2]; 
													dxx  =   dx-dx1;
													dyy	 =	 dy-dy1;
													dzz	 =   dz-dz1;
													distance =sqrt(pow(dxx,2)+pow(dyy,2)+pow(dzz,2));

													if (distance >10&&distance<150)//輸出符合開爪距離點
													{
													Move_dataOutput7(chosen_uvpnt_F[0], chosen_uvpnt_F[1], chosen_uvpnt_F[2], point_count);
													Move_dataOutput7(interference[0], interference[1], interference[2],point_count);
													Move_dataOutput7(X, Y, Z,point_count);
													}
													if (point_count == 7)
													{
														int fsf;
														fsf = 1000;
													}
													for (int i = 0, j = 0; j < problem_points_count; i = i + 3, j++)
													{
														Move_dataOutput6(Vn[i], Vn[i + 1], Vn[i + 2], point_count);
													}
													couut++;
													Vn.RemoveAll();
												}
												if (problem_points_count == 0)
												{
													Move_dataOutput(chosen_uvpnt_F[0], chosen_uvpnt_F[1], chosen_uvpnt_F[2], 0);
													Move_dataOutput(interference[0], interference[1], interference[2], 0);
													Move_dataOutput(X, Y, Z, 0);
													/////////////////////////////////////////////////////////////////
													double dx,dy,dz,dx1,dy1,dz1,dxx,dyy,dzz,distance;  
													dx   =   chosen_uvpnt_F[0]; 
													dy   =   chosen_uvpnt_F[1]; 
													dz   =   chosen_uvpnt_F[2]; 
													dx1  =   interference[0]; 
													dy1  =   interference[1]; 
													dz1  =   interference[2]; 
													dxx  =   dx-dx1;
													dyy	 =	 dy-dy1;
													dzz	 =   dz-dz1;
													distance =sqrt(pow(dxx,2)+pow(dyy,2)+pow(dzz,2));
													if (distance >10&&distance<150)//輸出符合開爪距離點
													{
													Move_dataOutput7(chosen_uvpnt_F[0], chosen_uvpnt_F[1], chosen_uvpnt_F[2], point_count);
													Move_dataOutput7(interference[0], interference[1], interference[2],point_count);
													Move_dataOutput7(X, Y, Z,point_count);
													}
													couut++;
												}


											}
											else
											{
												problem_points_count = 0;
												Vn.RemoveAll();
											}
											depth_check = 0;
											problem_points_count = 0;
										}
										else
										{
											cover = 0;
											problem_points_count = 0;
											Vn.RemoveAll();
										}
									}
									else
									{
										problem_points_count = 0;
										Vn.RemoveAll();
									}
									if (Jaw_width_inspection1>0)
									{
										break;
									}
								}
							}
							else
							{
								problem_points_count = 0;
								Vn.RemoveAll();
							}
							////////////
						}
						

					}
					////for/////
				}
			}
		}
	}
	end = clock();
	double Times = double(end - begin) / CLOCKS_PER_SEC; //將clock()函數的結果轉化爲以秒爲單位的量
	FILE *pFile;
	int arrSize;
	pFile = fopen("time_robot.txt", "a");
	fprintf(pFile, "%f \n", Times);
	fclose(pFile);

	return err;
}

//函式定義_FunctionDefinition
ProError  SolidSurfacesGet(ProSolid solid,			//(In)當前模型
	ProSurface **p_data)		//(In)曲面對象數組
{
	ProError err;
	err = ProArrayAlloc(0, sizeof(ProSurface), 1, (ProArray*)p_data);
	ProSolidSurfaceVisit(solid, (ProSurfaceVisitAction)SurfaceVisitActionFn, NULL, (ProAppData)&p_data);
	return err;
}

ProError SurfaceVisitActionFn(ProSurface surface, ProError err, ProAppData app_data)
{
	ProArray *pArray;
	pArray = (ProArray*)((ProSurface **)app_data)[0];
	err = ProArrayObjectAdd(pArray, PRO_VALUE_UNUSED, 1, &surface);
	return err;
}
//void axisget(int a, int b, int c) {
//	char buff[BUFSIZ];
//
//	FILE *input = fopen("C:\\Users\\Ren\\Desktop\\0812\\catch_points_R\\axis.txt", "r+");
//	if (input == NULL)
//		perror("Error opening file");
//	else {
//
//		while (feof(input) == 0) {
//
//			while (fgets(buff, sizeof buff, input) != NULL) {
//				if (sscanf(buff, "%d %d %d", &a, &b, &c) == 3) {
//					printf("%d %d %d\n", a, b, c);
//				}
//			}
//		}
//		fclose(input);
//	}
//
//}
ProError AsmCompVisitAct(ProAsmcomppath *p_path, ProSolid handle, ProBoolean down, ProAppData app_data)
{
	if (down == PRO_B_FALSE) return PRO_TK_NO_ERROR;
	AsmCompData outData;
	ProArray *pArray;
	ProError err;
	ProMdlType mdlType;
	ProName    name;
	err = ProMdlTypeGet((ProMdl)handle, &mdlType);
	err = ProMdlNameGet((ProMdl)handle, name);
	if (mdlType == PRO_MDL_ASSEMBLY) return PRO_TK_NO_ERROR;

	outData.comp_path = *p_path;
	outData.prt_handle = handle;

	pArray = (ProArray*)((AsmCompData**)app_data)[0];
	err = ProArrayObjectAdd(pArray, PRO_VALUE_UNUSED, 1, &outData);

	return err;
}
void Move_dataOutput(double a, double b, double c,int d)
{
	FILE *pFile;
	errno_t err;
	err = fopen_s(&pFile, "OutPutData.txt", "a");
	if (pFile == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile, "%f %f %f %d \n", a, b, c, d);
		fclose(pFile);
	}
}
void Move_dataOutput1(double a, double b, double c)
{
	FILE *pFile1;
	errno_t errr;
	errr = fopen_s(&pFile1, "COD.txt", "a");
	if (pFile1 == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile1, "%f %f %f \n", a, b, c);
		fclose(pFile1);
	}
}
void Move_dataOutput2(double a, double b)
{
	FILE *pFile2;
	errno_t errrr;
	errrr = fopen_s(&pFile2, "check.txt", "a");
	if (pFile2 == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile2, "%f %f %f \n", a, b);
		fclose(pFile2);
	}
}
void Move_dataOutput3(int a)
{
	FILE *pFile2;
	errno_t errrr;
	errrr = fopen_s(&pFile2, "ID.txt", "a");
	if (pFile2 == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile2, "%d \n", a);
		fclose(pFile2);
	}
}
void Move_dataOutput4(double a, double b, double c)
{
	FILE *pFile;
	errno_t err;
	err = fopen_s(&pFile, "OutPutData2.txt", "a");
	if (pFile == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile, "%f %f %f \n", a, b, c);
		fclose(pFile);
	}
}

void Move_dataOutput5(double a, double b, double c)
{
	FILE *pFile1;
	errno_t errr;
	errr = fopen_s(&pFile1, "outout.txt", "a");
	if (pFile1 == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile1, "%f %f %f \n", a, b, c);
		fclose(pFile1);
	}
}
void Move_dataOutput6(double a, double b, double c,int d)
{
	FILE *pFile1;
	errno_t errr;
	errr = fopen_s(&pFile1, "Detection_direction.txt", "a");
	if (pFile1 == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile1, "%f %f %f %d\n", a, b, c, d);
		fclose(pFile1);
	}
}
void Move_dataOutput7(double a, double b, double c,int d)
{
	FILE *pFile7;
	errno_t err;
	err = fopen_s(&pFile7, "OutPutData(0).txt", "a");
	if (pFile7 == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile7, "%f %f %f %d \n", a, b, c, d);
		fclose(pFile7);
	}
}
void Move_dataOutput8(double a, double b, double c)
{
	FILE *pFile8;
	errno_t err;
	err = fopen_s(&pFile8, "OutPutData(1).txt", "a");
	if (pFile8 == NULL)
	{
		AfxMessageBox(_T("Export data Error"));
	}
	else
	{
		fprintf(pFile8, "%f %f %f %d \n", a, b, c);
		fclose(pFile8);
	}
}
ProError FeatureVisitActionFn(ProFeature *feature, ProError err, ProAppData app_data)
{
	ProMdl prtMdl;
	ProMdlType p_type;
	ProArray *pArray;
	pArray = (ProArray*)((ProSurface **)app_data)[0];
	err = ProAsmcompMdlGet(feature, &prtMdl);
	if (err != PRO_TK_NO_ERROR)return(PRO_TK_CONTINUE);
	err = ProMdlTypeGet(prtMdl, &p_type);

	if (p_type == PRO_MDL_PART)
	{
		err = ProArrayObjectAdd(pArray, PRO_VALUE_UNUSED, 1, feature);
	}
	else if (p_type == PRO_MDL_ASSEMBLY)
	{
		err = ProSolidFeatVisit((ProSolid)prtMdl, FeatureVisitActionFn, FeatureFilterActionFn, app_data);
	}
	if (feature != NULL) return(PRO_TK_NO_ERROR);
	return(PRO_TK_CONTINUE);
}
ProError getAsmFeature(ProMdl asm_mdl, ProFeature** appdata)
{
	ProError err;
	//分配可擴展的空陣列
	err = ProArrayAlloc(0, sizeof(ProFeature), 1, (ProArray*)appdata);

	err = ProSolidFeatVisit((ProSolid)asm_mdl, FeatureVisitActionFn,
		FeatureFilterActionFn, (ProAppData)&appdata);//取得ASM下所有的零件與次組件
	return err;
}
ProError getAsmPrt(ProMdl asm_mdl, AsmCompData** appdata)
{
	ProError err;
	//分配可擴展的空陣列
	err = ProArrayAlloc(0, sizeof(AsmCompData), 1, (ProArray*)appdata);
	err = ProSolidDispCompVisit((ProSolid)asm_mdl, AsmCompVisitAct, NULL, (ProAppData)&appdata);
	return err;
}
ProError FeatureFilterActionFn(ProFeature *feature, ProAppData app_data)
{
	ProFeattype FeatType;
	ProFeatureTypeGet(feature, &FeatType);
	if (FeatType == PRO_FEAT_COMPONENT) //過濾PRO_FEAT_COMPONENT以外的特徵
		return PRO_TK_NO_ERROR;
	else
		return PRO_TK_CONTINUE;
}