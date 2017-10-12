/********************************************************************
	created:	2016/07/09
	created:	
	filename: 	D:\cygwin64\home\ibm\prog\rtu\inc\iec101.h
	file path:	D:\cygwin64\home\ibm\prog\rtu\inc
	file base:	iec101
	file ext:	h
	author:	zxf	    
	purpose:	iec101 balance protocol
*********************************************************************/
#ifndef	_IEC101_H_
#define	_IEC101_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../rdb/rdb.h"

#define  IEC101_APPID 	  12404
#define  COMREGADDR       0x6868

#define   DEBUG     1

#define OK 	  0
#define ERROR -1
//控制域
#define DIR 0x80
#define PRM 0x40
#define FCB 0x20
#define ACD 0x20
#define FCV 0x10
#define DFC 0x10
#define P_N 0x40
#define T_S 0x80
#define FCB_RST0 0x00
#define FCB_RST1 0x20
#define FCB_RST2 0x30
//品质描述词
#define QDS_GOOD 0x00
#define QDS_OV 0x01
#define QDS_BL 0x10
#define QDS_SB 0x20
#define QDS_NT 0x40
#define QDS_IV 0x80

#define DBGOUT_NULL		0x00
#define DBGOUT_STDPNT	0x01
#define DBGOUT_DBGPNT	0x02
#define DBGOUT_DBGSKT	0x04


#define DATA_PRI_MAXNUM 25

#define SETPOINT_MAX_NUM 200

#define RPT_MAX_NUM 200
#define RPT_MID_NUM  60

#define INIT_NULL 0x00
#define INIT_FCBRST 0x01
#define INIT_GI 0x02
#define INIT_TIMESYN 0x04
#define INIT_LINKREPLY 0x08

#define LINK_OK 0x0b
#define LINK_NOWORK 0x0e
#define LINK_NOCOMPLETE 0x0f


#define DUBUGSTRMAXLEN 		500

#define YKCLOSENOCHK_NULL 	0x00000000
#define YKCLOSENOCHK_START 	0x00000001
#define YKCLOSENOCHK_END 	0x00000002


#define YES 1
#define NO 0
#define IEC101_MAX_RESEND_NUM 3

#define MAX_YX_EVENT_NUM_PER_FRAME_101	10
#define MAX_YC_EVENT_NUM_PER_FRAME_101	10

#define GPIO_RS485_RW			47
#define RS485_RD				0
#define RS485_WR				1

enum {ENUM_0=0,RDB_RET_DISCONN,RDB_RET_NORMAL};
enum {ENUM_1=0,COMMUCOT_INIT,COMMUCOT_RECV,COMMUCOT_TIMEOUT};
enum {ENUM_2=0,IEC101_V2002,CHN101_V1997};
enum {ENUM_3=0,MODE_UNBALANCE,MODE_BALANCE};
enum {ENUM_4=0,E5_INVALID,E5_ACK,E5_NODATA,E5_ACK_NODATA_ALL,E5_ACK_ALL,E5_NODATA_ALL,E5_ACK_NODATA_NODATA1,E5_ACK_NODATA1,E5_NODATA_NODATA1};
enum {ENUM_5=0,YCBH_JUDGE_SOLID,YCBH_JUDGE_PERCENT};
enum {ENUM_6=0,CTLMODE_Std_S_E,CTLMODE_Std_E,CTLMODE_REGIS};
enum {ENUM_7=0,PRI_INVALID=0,PRI_LINKINITEND,PRI_GLOBALI,PRI_GLOBALI_APPACK,PRI_GLOBALI_YX,PRI_GLOBALI_YC,PRI_GLOBALI_YP,PRI_GLOBALI_END,PRI_TOTALSQUERY,PRI_GROUPI,PRI_CLOCKSYN,PRI_COLLECTDELAY,PRI_YKYT,PRI_SETPOINT,PRI_COS,PRI_YCBH,PRI_YPBH,PRI_SOE,PRI_READ,PRI_APPTEST,PRI_RESET,PRI_DOWNLOADPARA,PRI_BACKGROUNDSCAN,PRI_PERIODICYC,PRI_UNKNOWNAPPMIRROR,PRI_FILETRANS,PRI_DATAPASS};
enum {ENUM_8=0,PRTMSG_RAW,PRTMSG_FRM};
enum {ENUM_9=0,LINKPROCESS,APPLICATIONPROCESS,SENDPROCESS};
enum {ENUM_10=0,COMR_NO_DATA,COMR_LONG_DATA,COMR_SHORT_DATA,COMR_SAFE_DATA};
enum {ENUM_11=0,DGLEN_YX_SINGLE,DGLEN_YX_CONTINUE,DGLEN_YX_ASDU20,DGLEN_SOE,DGLEN_YC_CONTINUE,DGLEN_YC_SINGLE_NOTIME,DGLEN_YC_SINGLE_NOTIME_ASDU9,DGLEN_YC_SINGLE_NOTIME_ASDU11,DGLEN_YC_SINGLE_NOTIME_ASDU13,DGLEN_YC_SINGLE_NOTIME_ASDU21,DGLEN_YC_SINGLE_TIME,DGLEN_YM_NOTIME,DGLEN_YM_TIME,DGLEN_YP_CONTINUE,DGLEN_YP_SINGLE};
enum {ENUM_12=0,DEBUG_CHINESE,DEBUG_ENGLISH};
enum {ENUM_13=0,GIDATA_SYX,GIDATA_DYX,GIDATA_YC,GIDATA_YP};
enum {ENUM_14=0,GSDATA_SYX,GSDATA_DYX,GSDATA_YC,GSDATA_YP};
enum {ENUM_15=0,GROUP_INVALID,GROUP_SYX,GROUP_DYX,GROUP_YC,GROUP_YP};
enum {ENUM_16=0,CMD_NULL,CMD_SYK,CMD_DYK,CMD_YT,CMD_SP};
enum {ENUM_17=0,YKYT_REPLY_ACK,YKYT_REPLY_NAK,YKYT_REPLY_NAK_COTERR,YKYT_REPLY_NAK_APPADDERR,YKYT_REPLY_NAK_INFADDERR,YKYT_REPLY_SELECT,YKYT_REPLY_EXECUTE,YKYT_REPLY_CANCEL,YKYT_REPLY_END};
enum {ENUM_18=0,YKYT_STEP_NULL,YKYT_STEP_SELECT_CMD,YKYT_STEP_SELECT_BACK,YKYT_STEP_EXECUTE_CMD,YKYT_STEP_EXECUTE_BACK,YKYT_STEP_CANCEL_CMD,YKYT_STEP_CANCEL_BACK};
enum {ENUM_19=0,INIT_STEP_NULL,INIT_STEP_LINKREPLY,INIT_STEP_FCBRST,INIT_STEP_GI,INIT_STEP_YMI,INIT_STEP_TIMESYN,INIT_STEP_OVER};
enum {ENUM_20=0,TIMEDIFF_INVALID,TIMEDIFF_SYX,TIMEDIFF_DYX,TIMEDIFF_YC,TIMEDIFF_YXYC_MUST,TIMEDIFF_YXYC_NOSAME};
enum {ENUM_21=0,COSSEND_NO,COSSEND_LINK,COSSEND_INIT,COSSEND_FIRSTGI};
enum {ENUM_22=0,SOESEND_NO,SOESEND_LINK,SOESEND_INIT,SOESEND_FIRSTGI};
enum {ENUM_23=0,DEBUGTIME_NULL,DEBUGTIME_DATE,DEBUGTIME_TIME,DEBUGTIME_DATE_TIME,DEBUGTIME_STD_DATE_TIME,DEBUGTIME_Y_M_D_H_M_S};
enum {ENUM_24=0,ACD_STD,ACD_CHINANORTH};
enum {ENUM_25=0,COT_IEC,COT_VALID_INVALID};
enum {ENUM_26=0,ASDUCHK_NULL,ASDUCHK_ADDR,ASDUCHK_VSQ,ASDUCHK_COT,ASDUCHK_VSQ_ADDR,ASDUCHK_COT_ADDR,ASDUCHK_VSQ_COT_ADDR};
enum {ENUM_27=0,ASDU70_VALID_NULL,ASDU70_VALID_FCB,ASDU70_VALID_L_FCB,ASDU70_VALID_R_FCB,ASDU70_VALID_LR_FCB};
enum {ENUM_28=0,TIMESYN_MASTER,TIMESYN_MASTER_STATIC_OFFSET,TIMESYN_MASTER_DYNAMIC_OFFSET,TIMESYN_SLAVE_PRE,TIMESYN_SLAVE_POST};
enum {ENUM_29=0,YKYTOPEN_FOREVER,YKYTOPEN_RL};
enum {ENUM_30=0,BYTEORDER_LITTLE,BYTEORDER_BIG};
enum {ENUM_31=0,YKYTOVER_ALWAYS,YKYTOVER_NO,YKYTOVER_SUCESS,YKYTOVER_FAIL};
enum {ENUM_32=0,YCBH_SEND_LINKINIT,YCBH_SEND_COMMINIT,YCBH_SEND_FIRSTGI,YCBH_SEND_NO};
enum {ENUM_33=0,YPBH_SEND_LINKINIT,YPBH_SEND_COMMINIT,YPBH_SEND_FIRSTGI,YPBH_SEND_NO};
enum {ENUM_34=0,LINKPREINIT_NOANSWER,LINKPREINIT_NOWORK,LINKPREINIT_NOCOMPLETE,LINKPREINIT_NOWORK1,LINKPREINIT_NOWORK2};
enum {ENUM_35=0,YC_MODIFY_NULL,YC_MODIFY_SHORT_FLOW};
enum {ENUM_36=0,CLRBUF_PROCRST,CLRBUF_FCBRST,CLRBUF_SWITCHDOWN,CLRBUF_NOCLEAR,CLRBUF_ALL,CLRBUF_BUTCOS,CLRBUF_BUTSOE,CLRBUF_BUTCOSSOE};
enum {ENUM_37=0,COTMATCH_NULL,COTMATCH_LOW4BIT,COTMATCH_LOWBYTE};
enum {ENUM_38=0,SETPOINT_NULL,SETPOINT_SELECT_READY,SETPOINT_SELECT,SETPOINT_SELECT_BACK,SETPOINT_EXECUTE_READY,SETPOINT_EXECUTE,SETPOINT_EXECUTE_BACK,SETPOINT_CANCEL_READY,SETPOINT_CANCEL,SETPOINT_CANCEL_BACK};
enum {ENUM_39=0,SIGRST_NULL,SIGRST_YK_SEL_EXE,SIGRST_YK_SEL1,SIGRST_YK_SEL2,SIGRST_YK_EXE1,SIGRST_YK_EXE2,SIGRST_YK_ONLY_INDEX1,SIGRST_YK_ONLY_INDEX2};
enum {ENUM_40=0,INFADDR_CONTINUE,INFADDR_SINGLE};
enum {ENUM_41=0,STD_YC,STD_YX,STD_SYX,STD_DYX,STD_YM,STD_YT,STD_YP,STD_SYK,STD_DYK};
enum {ENUM_42=0,YXSTATE_OPN_CLS,YXSTATE_OPN_CLS_IV,YXSTATE_OPN_CLS_IV_CNSHENZHEN,YXSTATE_OPN_CLS_MID_IV,YXSTATE_OPN_CLS_MID,YXSTATE_OPN_CLS_MID_QDS,YXSTATE_OPN_CLS_QDS};
enum {ENUM_43=0,FCB_REPLY_NORMAL,FCB_REPLY_NO,FCB_REPLY_RESEND_ALWAYS,FCB_REPLY_RESEND_3};
enum {ENUM_44=0,YCBH_JUDGE_IDENTICAL,YCBH_JUDGE_INDEPENDENT};

enum {ENUM_46=0,FCBPOST_0,FCBPOST_1,FCBPOST_FUZZY};
enum {ENUM_47=0,YKCLOSEMODE_AUTO_ALWAYS,YKCLOSEMODE_NOCHK_ALWAYS,YKCLOSEMODE_SYNCHK_ALWAYS,YKCLOSEMODE_VOLTCHK_ALWAYS,YKCLOSEMODE_SYNCHK_DUMMYBI0,YKCLOSEMODE_SYNCHK_DUMMYBI1,YKCLOSEMODE_VOLTCHK_DUMMYBI0,YKCLOSEMODE_VOLTCHK_DUMMYBI1,YKCLOSEMODE_NOCHK_DUMMYBI0,YKCLOSEMODE_NOCHK_DUMMYBI1,YKCLOSEMODE_NARIDEFINE,YKCLOSEMODE_NOCHK_FOSHAN,YKCLOSEMODE_JUMPDEFINE};
enum {ENUM_48=0,FTRANS_RPT_DIR,FTRANS_RPT_FILE_CONTENT,FTRANS_RPT_FILE_READY,FTRANS_RPT_SECTION_READY,FTRANS_RPT_SECTION_ACK};
enum {ENUM_49=0,FTRANS_FSN_1B,FTRANS_FSN_2B,FTRANS_FSN_3B,FTRANS_FSN_4B};
enum {ENUM_50=0,FTRANS_FFIND_LOCAL};
enum {ENUM_51=0,YCVALVE_IDENTICAL,YCVALVE_INDEPENDENT};
enum {ENUM_52=0,YCVALVE_NODEAL,YCVALVE_VAL_FLOW,YCVALVE_VAL_NOFLOW,YCVALVE_0_FLOW,YCVALVE_0_NOFLOW};
enum {ENUM_53=0,YXDELAY_NULL,YXDELAY_NO,YXDELAY_TOON,YXDELAY_TOOFF};
enum {ENUM_54=0,PROCRST_NOOPER,PROCRST_IEC,PROCRST_CLEARBUF,PROCRST_RSTPROC,PROCRST_RSTRTU};
enum {ENUM_55=0,READYCLIST_READ,READYCLIST_CHECK};
enum {ENUM_56=0,YKYTLOCK_NO,YKYTLOCK_IDENTICAL,YKYTLOCK_INDEPENDENT,YKYTLOCK_IDCAL_IDEPEN};
enum {ENUM_57=0,YKYTLOCK_ST_OFF,YKYTLOCK_ST_ON,YKYTLOCK_ST_MID,YKYTLOCK_ST_IV,YKYTLOCK_ST_OFF_IV,YKYTLOCK_ST_ON_IV,YKYTLOCK_ST_MID_IV};
enum {ENUM_58=0,GRAM_NULL,GRAM_YX_GLOBALI,GRAM_YX_GROUPI,GRAM_YX_BACKGROUND,GRAM_YC_GLOBALI,GRAM_YC_GROUPI,GRAM_YC_BACKGROUND,GRAM_YP_GLOBALI,GRAM_YP_GROUPI,GRAM_YP_BACKGROUND};

enum {ENUM_60=0,GINEW_REPONSE_FIFO,GINEW_STOPOLD_REPONSENEW,GINEW_CONTINUEOLD_IGNORENEW,GINEW_CONTINUEOLD_NAKNEW};
enum {ENUM_61=0,COTFIG_NORMAL,COTFIG_NAK,COTFIG_ACK};
enum {ENUM_62=0,GROUPINODATA_APPNAK,GROUPINODATA_APPACKEND};
enum {ENUM_63=0,LINKILCTL_NOANSWER,LINKILCTL_NOWORK,LINKILCTL_NOCOMPLETE};
enum {ENUM_64=0,INITRULE_DEFINE,INITRULE_OPENYKYT};
enum {ENUM_65=0,LIST_SYX,LIST_DYX,LIST_YC,LIST_SYK,LIST_DYK,LIST_YM,LIST_YP,LIST_YT};
enum {ENUM_66=0,CHGSRC_DOWN,CHGSRC_DOWN_RDB,CHGSRC_DOWN_COMMU,CHGSRC_DOWN_RDB_COMMU};
enum {ENUM_67=0,LINKRULE_IEC,LINKRULE_DONGFANG, LINKRULE_ZHUHAI_XUJI};
enum {ENUM_68=0,DIR_MASTER_1,DIR_MASTER_0};
enum {ENUM_69=0,REPORT_50MS,REPORT_100MS,REPORT_200MS,REPORT_400MS,REPORT_600MS,REPORT_800MS,REPORT_1000MS,REPORT_1400MS,REPORT_AUTOMATCH};

typedef  unsigned char  u_char;
typedef  unsigned int   u_int;
typedef  unsigned int   u_int32;
typedef  unsigned short u_int16;
typedef  unsigned char  u_int8;

typedef  int            int32;
typedef  short          int16;
typedef  char           int8;
//报文记录
struct MSGREPORT
{
	u_int8 buf[300];
	u_int16 len;
	struct MSGREPORT * next;
};

struct APPYKDEF
{
	u_int8 cmd;
	u_int8 step;
	u_int8 asdutype;
	u_int8 rece_vsq;
	u_int16 rece_cot;
	u_int16 rece_appaddr;
	u_int32 rece_Infaddr;
	u_int8 rece_dco;
	u_int8 vsq;						//可变结构限定词
	u_int16 cot;					//传送原因
	u_int16 appaddr;			//公共地址
	u_int32 infaddr;			//信息对象地址
	u_int8 dco;
	u_int8 dco_bak;
	u_int8 dco_old;
	u_int16 point;
	u_int32 index;
	u_int32 index_bak;
	u_int32 index2;
	u_int32 Yk_TimeCount;
	u_int32 Yt_TimeCount;
	u_int8 ResultFig;
	u_int32 dest_appid;
	u_int32 return_id;//命令发布者区分命令类型
	u_int8 byCommandStage;//遥控命令 	0:选择 1:执行 2:取消
	u_int8 byActionType;//遥控动作	0:分; 1:合
	u_int8 byYKType;//遥控类型	RDB_YKTYPE_YK=遥控,RDB_YKTYPE_BSC=调档,RDB_YKTYPE_SETPOINT=遥调设置,RDB_YKTYPE_RSC=复归
	u_int8 bySynType;//遥控同期类型	0:不检; 1:检同期; 2:检无压; 3:自适应;4:接地试跳;5:检合环
	u_int8 ocLock;//是否进行遥控五防校验的标志位,0：不需要五防校验,1：需要进行五防校验

};

struct ASDUHEAD {
	u_int8 asdutype;
	u_int8 vsq;
	u_int16 cot;
	u_int16 appaddr;
};

struct APPDEF
{
	yx_event_t * yx_event_lst;
	int yx_event_num;
	yc_event_t * yc_event_lst;
	int yc_event_num;

	int     com_no;
	int		rs_mode;		/* 485 or 232 */
	int	app_id;
	char app_name[32];
	int    udp_server_id;
	int    udp_client_id;
	int	comfd;

	u_int8 rbuf[1000];
	u_int8 sbuf[300];
	u_int8 sbuf2[300];
	u_int8 sbuf3[300];
	u_int8 ReceSynFig;
	u_int16 ReceBytes;
	u_int16 rlen,slen,slen2,slen3,rptr,min_len;
	u_int8 rfig,fcb,acd,ctrl;
	u_int8 datastep_fig;
	struct MSGREPORT * ReportPtr[RPT_MAX_NUM];
	struct MSGREPORT * data1ptr[DATA_PRI_MAXNUM];
	struct MSGREPORT * data2ptr[DATA_PRI_MAXNUM];
	u_int16 start_time,commout_time,linkaddr_bytes,link_addr,appaddr_bytes,appaddr;
	u_int16 cot_bytes,infaddr_bytes,telegram_MAXbytes,rece_appaddr;
	u_int8 E5_strategy,E5_FcbRstValid,ACD_FcbRstValid;
	u_int8 LinkACD_strategy,ASDUCHK_strategy,COT_strategy;
	u_int8 TimeDiff_AlertFig,ASDU70ValidFig,Data1RespondRequest2Fig;
	u_int8 ASDU70ValidStrategy,InitLocationFig,TimeSynStrategyFig;
	u_int16 ReceSourceCount,YCUpMaxValue,TimeDynamicOffset;
	u_int32 infaddr_TimeDiff,rece_infaddr;
	u_int TimeDiff_YxIndex,TimeDiff_YcIndex;
	float TimeDiff_valve;
	u_int8 InitOverFig,InitStepFig,InitStepFig2;
	u_int8 InitPeriodRuleFig,FloatYcByteFig,IntYcByteFig;
	u_int8 CommErrSaveSbiCOSFig,CommErrSaveDbiCOSFig,CommErrSaveSSOEFig,CommErrSaveDSOEFig;
	u_int8 ASDU_YC_GI,ASDU_YX_GI,ASDU_YP_GI,ASDU_YM_PI,ASDU_YC_GroupI;
	u_int8 ASDU_YX_GroupI,ASDU_YP_GroupI,ASDU_YC_ReadI,ASDU_SYX_ReadI,ASDU_DYX_ReadI,ASDU_YP_ReadI,ASDU_YM_ReadI;
	u_int8 YCBH_SendStrategy,YPBH_SendStrategy,YCBH_JudgeType,COS_SendFig,SOE_SendFig;
	u_int8 YcBh_ReportStrategy,YpBh_ReportStrategy,COS_ReportStrategy,SOE_ReportStrategy;	
	u_int8 ASDU_YCBH,ASDU_COS,ASDU_YPBH,ASDU_SSOE,ASDU_DSOE;
	u_int8 ASDU_COS_TYPE,ASDU_SOE_TYPE;
	float YCBH_judge_valve;
	u_int16 COSNumPerFrame,SOENumPerFrame,YCBHNumPerFrame,YPBHNumPerFrame;
	u_int8 SYK_cmdexe_mode,DYK_cmdexe_mode,ASDU_SYK,ASDU_DYK,YT_cmdexe_mode,ASDU_YT;
	u_int8 SYkOverStgy,DYkOverStgy,YtOverStgy;
	u_int8 BgdScanfig,ASDU_YC_BackgroundScan,ASDU_YX_BackgroundScan,ASDU_YP_BackgroundScan;
	u_int16 BgdScanPrdTime;
	u_int8 PeriodicScanYcFig,ASDU_YC_PeriodicScan;
	u_int16 PeriodicScanYcTime;
	u_int32 InfAddr_syx,InfAddr_dyx,InfAddr_yc,InfAddr_ym,InfAddr_yp;
	u_int32 InfAddr_syk,InfAddr_dyk,InfAddr_yt;
	u_int32 InfAddr_PeriodicYcBegin,InfAddr_PeriodicYcEnd;
	u_int32 InfAddr_GroupI_begin[16],InfAddr_GroupI_end[16],InfAddr_TotalsI_begin[4],InfAddr_TotalsI_end[4];
	u_int8 Pri_Data1[DATA_PRI_MAXNUM],Pri_Data2[DATA_PRI_MAXNUM];
	u_int32 second_count,comm_count,comm_count2;
	u_int8 comm_status;
	u_int8 start_fig;
	u_int8 GroupType[16];
	u_int8 testbuf[10];
	struct APPYKDEF YkYt;
	u_int32 SYK_TimeOut,DYK_TimeOut,YT_TimeOut;
	u_int8 Version101,CommuMode;
	u_int8 GlobalIReplyFig,GroupIReplyFig,YmReplyFig;
	u_int8 DebugTimeFig;
	u_int8 PrintMsgFig;
	u_int8 resendfig;
	u_int8 LinkAnswerStgy;
	u_int8 LinkIlctlStgy;
	u_int8 DebugLanguage;
	u_int8 DebugOutFig;
	u_int8 GlobalI_DataSeqFig[4];
	u_int8 BgdScanDataSeq[4];
	u_int16 protocol_no,rll_index;
	//struct APPSETPOINTBASEDEF SpBase;
	//struct APPSETPOINTDEF SetPoint[SETPOINT_MAX_NUM];
	u_int8 YmI_fig,GlobalI_fig,GroupI_fig;
	u_int8 YmI_qcc,GroupI_qoi;
	u_int8 ReceSourceFig,ReceSourceBakFig;
	u_int8 YcModifyFig,InitOkFig;
	u_int16 Chk200ms_count;
	u_int8 CotMatch_strategy;
	u_int8 SigRstStgy;
	u_int32 SigRstIndex;
	u_int8 YxSynFig,ReportSwichFig;
	u_int8 SYxInfaddrType,YcInfaddrType,DYxInfaddrType,YmInfaddrType;
	u_int8 YtInfaddrType,YpInfaddrType,SYkInfaddrType,DYkInfaddrType;
	u_int8 GlbISyxStgy,GlbIDyxStgy;
	u_int8 GrpISyxStgy,GrpIDyxStgy;
	u_int8 SolISyxStgy,SolIDyxStgy;
	u_int8 BgdSyxStrategy,BgdDyxStrategy;
	u_int8 ChgSyxSrcFig,ChgDyxSrcFig;
	u_int8 ChgSyxStFig,ChgDyxStFig;
	u_int8 FcbReplyStrategy;
	u_int16 FcbCount;
	u_int32 TimeStaticOffset;
	u_int8 ProtoEnableFig;
	char comm_name[100];
	u_int8 YCBH_JudgeMode;
	u_int32 scc_count;
	u_int8 FcbPostState;
	u_int8 FcbStartFig;
	u_int8 CHN101AppFig;
	char TaskName[100];
	u_int8 YkCloseMode_Strategy;
	u_int32 YkCloseMode_YkIndex;
	u_int32 YkCloseMode_YxIndex;
	u_int32 YkCloseMode_count;
	u_int32 YkCloseMode_ValidTime;
	//struct FILETRANS_STRUCT ftrans;
	u_int32 FnChkCount;
	//struct YCLIMIT_STRUCT YcLimit;
	u_int8 ProcRstStgy;
	u_int32 CtlCmdInfTransmitFig;
	u_int32 RstRtuCount;

	u_int32 YkCloseNoChkOperFig;
	u_int16 YxNumPerFrmGlobalI;
	u_int16 YcNumPerFrmGlobalI;
	u_int16 YpNumPerFrmGlobalI;
	u_int8 GlobalI_NewStrategy;
	u_int16 YxNumPerFrmGroupI;
	u_int16 YcNumPerFrmGroupI;
	u_int16 YpNumPerFrmGroupI;
	u_int8 GroupNullReplyStgy;
	u_int16 YxNumPerFrmBgd;
	u_int16 YcNumPerFrmBgd;
	u_int16 YpNumPerFrmBgd;
	u_int8 LnkRstBufferFig;
	u_int32 hookcount;
	u_int32 scos_count;
	u_int32 dcos_count;
	u_int32 ssoe_count;
	u_int32 dsoe_count;
	int HeadLen;
	u_int8 DirStrategy;
	u_int8 AutoTest;
	u_int8 dirfig;	
	u_int32 TestCount;
	u_int32 fresendframe2;
	u_int32 fsendcount2;
	u_int8   fsendfig2;	
	u_int8 fcb2;
	u_int8 LnkFstInitOkFig;
	u_int8 fresendfig2;
	u_int8 LnkInitStartFig2;	
	u_int8 LnkRuleType;
};

struct SYS_TIME
{
	u_int16 msecond;
	u_int8  second;
	u_int8  minute;
	u_int8  hour;
	u_int8  day;
	u_int8  month;
	u_int8  unuse; /*保证为偶数*/
	u_int16 year;
};

extern int iec101_comm_stat;

time_t 	sys_time_to_timet(struct SYS_TIME *t);
int sys_get_time(struct SYS_TIME *sys_t);
int sys_open_com(struct APPDEF *dp, int databits, int stopbits, int parity, int baudrate, int com_no);
int send_iec101_internal_udp_data (unsigned char * send_buf, int send_len);
int init_iec101_service ();

#ifdef __cplusplus
}
#endif

#endif






