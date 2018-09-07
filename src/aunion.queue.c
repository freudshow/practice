/****************************************Copyright (c)**************************************************
 **                               广州周立功单片机发展有限公司
 **                                     研    究    所
 **                                        产品一部
 **
 **                                 http://www.zlgmcu.com
 **
 **--------------文件信息--------------------------------------------------------------------------------
 **文   件   名: queue.c
 **创   建   人: 陈明计
 **最后修改日期: 2003年7月2日
 **描        述: 数据队列的中间件
 **
 **--------------历史版本信息----------------------------------------------------------------------------
 ** 创建人: 陈明计
 ** 版  本: v1.0
 ** 日　期: 2003年7月2日
 ** 描　述: 原始版本
 **
 **--------------当前版本修订------------------------------------------------------------------------------
 ** 修改人: 陈明计
 ** 日　期: 2004年5月19日
 ** 描　述: 改正注释错误和常量引用错误
 **
 **------------------------------------------------------------------------------------------------------
 ** 修改人:Nichenqiang
 ** 修改: 添加函数QueueNotEnoughPend ,QueueRead_Int,QueueWrite_Int,QueueWrite_Int_f
 ** 特别说明: 关于函数QueueNData，QueueNotEnoughPend,应该注意，多任务环境下，调用函数获得结果后，
 ** NData可能会被其他任务或中断的队列操作修改，除非在整个获取和使用过程中关中断
 ** 日　期:2010年2月23日 ; 2010年3月13日; 2010年3月24日;
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/

#define IN_QUEUE
#include <includes.h>

/*********************************************************************************************************
 ** 函数名称: QueueCreate
 ** 功能描述: 初始化数据队列
 ** 输　入: Buf      ：为队列分配的存储空间地址
 **         SizeOfBuf：为队列分配的存储空间大小（字节）
 **         ReadEmpty：为队列读空时处理程序
 **         WriteFull：为队列写满时处理程序
 ** 输　出: NOT_OK:参数错误
 **         QUEUE_OK:成功
 ** 全局变量: 无
 ** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
 **
 ** 作　者: 陈明计
 ** 日　期: 2003年7月2日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
uint8 QueueCreate(void *Buf,                                 //倪 可重入
		uint32 SizeOfBuf, uint8 (*ReadEmpty)(), uint8 (*WriteFull)())
{
	OS_CPU_SR cpu_sr;
	DataQueue *Queue;

	if (Buf != NULL && SizeOfBuf >= (sizeof(DataQueue))) /* 判断参数是否有效 */
	{
		Queue = (DataQueue *) Buf;

		OS_ENTER_CRITICAL();
		/* 初始化结构体数据 */
		Queue->MaxData = (SizeOfBuf - (uint32)(((DataQueue *) 0)->Buf))
				/ sizeof(QUEUE_DATA_TYPE); /* 计算队列可以存储的数据数目 */
		Queue->End = Queue->Buf + Queue->MaxData; /* 计算数据缓冲的结束地址 */ //倪注 这里End指向最后一个可用字节的下一个字节
		Queue->Out = Queue->Buf;
		Queue->In = Queue->Buf;
		Queue->NData = 0;
		Queue->ReadEmpty = ReadEmpty;
		Queue->WriteFull = WriteFull;

		OS_EXIT_CRITICAL();

		return QUEUE_OK;
	} else {
		return NOT_OK;
	}
}

/*********************************************************************************************************
 ** 函数名称: QueueRead
 ** 功能描述: 获取队列中的数据
 ** 输　入: Ret:存储返回的消息的地址
 **         Buf:指向队列的指针
 ** 输　出: NOT_OK     ：参数错误
 **         QUEUE_OK   ：收到消息
 **         QUEUE_EMPTY：无消息
 ** 全局变量: 无
 ** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
 **
 ** 作　者: 陈明计
 ** 日　期: 2003年7月2日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
uint8 QueueRead(QUEUE_DATA_TYPE *Ret, void *Buf) //倪 可重入
{
	uint8 err;
	OS_CPU_SR cpu_sr;
	DataQueue *Queue;

	err = NOT_OK;
	if (Buf != NULL) /* 队列是否有效 */
	{ /* 有效 */
		Queue = (DataQueue *) Buf;

		OS_ENTER_CRITICAL();

		if (Queue->NData > 0) /* 队列是否为空 */
		{ /* 不空         */
			*Ret = Queue->Out[0]; /* 数据出队     */
			Queue->Out++; /* 调整出队指针 */
			if (Queue->Out >= Queue->End) {
				Queue->Out = Queue->Buf;
			}
			Queue->NData--; /* 数据减少      */
			err = QUEUE_OK;
		} else { /* 空              */
			err = QUEUE_EMPTY;
			if (Queue->ReadEmpty != NULL) /* 调用用户处理函数 */
			{
				err = Queue->ReadEmpty(Ret, Queue);
			}
		}
		OS_EXIT_CRITICAL();
	}
	return err;
}

/*********************************************************************************************************
 ** 函数名称: QueueRead_Int
 ** 功能描述: 获取队列中的数据,此函数只能在关中断状态下调用，与函数QueueRead的区别仅是去掉了关开中断
 ** 输　入: Ret:存储返回的消息的地址
 **         Buf:指向队列的指针
 ** 输　出: NOT_OK     ：参数错误
 **         QUEUE_OK   ：收到消息
 **         QUEUE_EMPTY：无消息
 ** 全局变量: 无
 ** 调用模块:
 **
 ** 作　者: Nichenqiang
 ** 日　期: 2010年3月13日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
uint8 QueueRead_Int(QUEUE_DATA_TYPE *Ret, void *Buf) //倪 可重入
{
	uint8 err;
	DataQueue *Queue;

	err = NOT_OK;
	if (Buf != NULL) /* 队列是否有效 */
	{ /* 有效 */
		Queue = (DataQueue *) Buf;

		//OS_ENTER_CRITICAL();

		if (Queue->NData > 0) /* 队列是否为空 */
		{ /* 不空         */
			*Ret = Queue->Out[0]; /* 数据出队     */
			Queue->Out++; /* 调整出队指针 */
			if (Queue->Out >= Queue->End) {
				Queue->Out = Queue->Buf;
			}
			Queue->NData--; /* 数据减少      */
			err = QUEUE_OK;
		} else { /* 空              */
			err = QUEUE_EMPTY;
			if (Queue->ReadEmpty != NULL) /* 调用用户处理函数 */
			{
				err = Queue->ReadEmpty(Ret, Queue);
			}
		}
		//OS_EXIT_CRITICAL();
	}
	return err;
}

/*********************************************************************************************************
 ** 函数名称: QueueWrite
 ** 功能描述: FIFO方式发送数据
 ** 输　入: Buf :指向队列的指针
 **         Data:消息数据
 ** 输　出: NOT_OK   :参数错误
 **         QUEUE_FULL:队列满
 **         QUEUE_OK  :发送成功
 ** 全局变量: 无
 ** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
 **
 ** 作　者: 陈明计
 ** 日　期: 2003年7月2日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
#ifndef EN_QUEUE_WRITE
#define EN_QUEUE_WRITE      0
#endif

#if EN_QUEUE_WRITE > 0

uint8 QueueWrite(void *Buf, QUEUE_DATA_TYPE Data) //倪 可重入
{
	uint8 err;
	OS_CPU_SR cpu_sr;
	DataQueue *Queue;

	err = NOT_OK;
	if (Buf != NULL) /* 队列是否有效 */
	{
		Queue = (DataQueue *)Buf;

		OS_ENTER_CRITICAL();

		if (Queue->NData < Queue->MaxData) /* 队列是否满  */
		{ /* 不满        */
			Queue->In[0] = Data; /* 数据入队    */
			Queue->In++; /* 调整入队指针*/
			if (Queue->In >= Queue->End)
			{
				Queue->In = Queue->Buf;
			}
			Queue->NData++; /* 数据增加    */
			err = QUEUE_OK;
		}
		else
		{ /* 满           */
			err = QUEUE_FULL;
			if (Queue->WriteFull != NULL) /* 调用用户处理函数 */
			{
				err = Queue->WriteFull(Queue, Data, Q_WRITE_MODE);
			}
		}
		OS_EXIT_CRITICAL();
	}
	return err;
}

/*********************************************************************************************************
 ** 函数名称: QueueWrite_Int
 ** 功能描述: FIFO方式发送数据,此函数只能在关中断状态下调用，与函数QueueWrite的区别仅是去掉了关开中断
 ** 输　入: Buf :指向队列的指针
 **         Data:消息数据
 ** 输　出: NOT_OK   :参数错误
 **         QUEUE_FULL:队列满
 **         QUEUE_OK  :发送成功
 ** 全局变量: 无
 ** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
 **
 ** 作　者: Nichenqiang
 ** 日　期: 2010年3月13日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
uint8 QueueWrite_Int(void *Buf, QUEUE_DATA_TYPE Data) //倪 可重入
{
	uint8 err;
	DataQueue *Queue;

	err = NOT_OK;
	if (Buf != NULL) /* 队列是否有效 */
	{
		Queue = (DataQueue *)Buf;

		//OS_ENTER_CRITICAL();

		if (Queue->NData < Queue->MaxData) /* 队列是否满  */
		{ /* 不满        */
			Queue->In[0] = Data; /* 数据入队    */
			Queue->In++; /* 调整入队指针*/
			if (Queue->In >= Queue->End)
			{
				Queue->In = Queue->Buf;
			}
			Queue->NData++; /* 数据增加    */
			err = QUEUE_OK;
		}
		else
		{ /* 满           */
			err = QUEUE_FULL;
			if (Queue->WriteFull != NULL) /* 调用用户处理函数 */
			{
				err = Queue->WriteFull(Queue, Data, Q_WRITE_MODE);
			}
		}
		//OS_EXIT_CRITICAL();
	}
	return err;
}
/*********************************************************************************************************
 ** 函数名称: QueueWrite_Int_f
 ** 功能描述: FIFO方式发送数据,此函数只在关中断状态的中断服务程序中调用，与函数QueueWrite的区别仅是去掉了关开中断,
 **           并去掉了参数的合法性判断,和返回值. 因此,特别注意,必须保证在中断使能前确保中断服务程序中使用的队列已经初始化.
 ** 输　入: Buf :指向队列的指针
 **         Data:消息数据
 ** 输　出:   无
 ** 全局变量: 无
 ** 调用模块:
 **
 ** 作　者: Nichenqiang
 ** 日　期: 2010年3月24日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
void QueueWrite_Int_f(void *Buf, QUEUE_DATA_TYPE Data)
{

	DataQueue *Queue;

	Queue = (DataQueue *)Buf;

	if (Queue->NData < Queue->MaxData) /* 队列是否满  */
	{ /* 不满        */
		Queue->In[0] = Data; /* 数据入队    */
		Queue->In++; /* 调整入队指针*/
		if (Queue->In >= Queue->End)
		{
			Queue->In = Queue->Buf;
		}
		Queue->NData++; /* 数据增加    */
	}

}
#endif

/*********************************************************************************************************
 ** 函数名称: QueueWriteFront
 ** 功能描述: LIFO方式发送数据
 ** 输　入: Buf:指向队列的指针
 **         Data:消息数据
 ** 输　出: QUEUE_FULL:队列满
 **         QUEUE_OK:发送成功
 ** 全局变量: 无
 ** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
 **
 ** 作　者: 陈明计
 ** 日　期: 2003年7月2日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
#ifndef EN_QUEUE_WRITE_FRONT
#define EN_QUEUE_WRITE_FRONT    0
#endif

#if EN_QUEUE_WRITE_FRONT > 0

uint8 QueueWriteFront(void *Buf, QUEUE_DATA_TYPE Data)
{
	uint8 err;
	DataQueue *Queue;

	err = NOT_OK;
	if (Buf != NULL) /* 队列是否有效 */
	{
		Queue = (DataQueue *)Buf;

		OS_ENTER_CRITICAL();

		if (Queue->NData < Queue->MaxData) /* 队列是否满  */
		{ /* 不满 */
			Queue->Out--; /* 调整出队指针 */
			if (Queue->Out < Queue->Buf)
			{
				Queue->Out = Queue->End - 1;
			}
			Queue->Out[0] = Data; /* 数据入队     */
			Queue->NData++; /* 数据数目增加 */
			err = QUEUE_OK;
		}
		else
		{ /* 满           */
			err = QUEUE_FULL;
			if (Queue->WriteFull != NULL) /* 调用用户处理函数 */
			{
				err = Queue->WriteFull(Queue, Data, Q_WRITE_FRONT_MODE);
			}
		}
		OS_EXIT_CRITICAL();
	}
	return err;
}

#endif

/*********************************************************************************************************
 ** 函数名称: QueueNData
 ** 功能描述: 取得队列中数据数
 ** 输　入: Buf:指向队列的指针
 ** 输　出: 消息数
 ** 全局变量: 无
 ** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
 **
 ** 作　者: 陈明计
 ** 日　期: 2003年7月2日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
#ifndef EN_QUEUE_NDATA
#define EN_QUEUE_NDATA    0
#endif

#if EN_QUEUE_NDATA > 0

uint16 QueueNData(void *Buf) //倪 可重入
{
	uint16 temp;
	OS_CPU_SR cpu_sr;

	temp = 0; /* 队列无效返回0 */
	if (Buf != NULL)
	{
		OS_ENTER_CRITICAL();
		temp = ((DataQueue *)Buf)->NData;
		OS_EXIT_CRITICAL();
	}
	return temp;
}

#endif

/*********************************************************************************************************
 ** 函数名称: QueueSize
 ** 功能描述: 取得队列总容量
 ** 输　入: Buf:指向队列的指针
 ** 输　出: 队列总容量
 ** 全局变量: 无
 ** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
 **
 ** 作　者: 陈明计
 ** 日　期: 2003年7月2日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
#ifndef EN_QUEUE_SIZE
#define EN_QUEUE_SIZE    0
#endif

#if EN_QUEUE_SIZE > 0

uint16 QueueSize(void *Buf) //倪 可重入
{
	uint16 temp;
	OS_CPU_SR cpu_sr;

	temp = 0; /* 队列无效返回0 */
	if (Buf != NULL)
	{
		OS_ENTER_CRITICAL();
		temp = ((DataQueue *)Buf)->MaxData;
		OS_EXIT_CRITICAL();
	}
	return temp;
}

#endif

/*********************************************************************************************************
 ** 函数名称: OSQFlush
 ** 功能描述: 清空队列
 ** 输　入: Buf:指向队列的指针
 ** 输　出: 无
 ** 全局变量: 无
 ** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
 **
 ** 作　者: 陈明计
 ** 日　期: 2003年7月2日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
#ifndef EN_QUEUE_FLUSH
#define EN_QUEUE_FLUSH    0
#endif

#if EN_QUEUE_FLUSH > 0

void QueueFlush(void *Buf) //倪 可重入
{
	OS_CPU_SR cpu_sr;
	DataQueue *Queue;

	if (Buf != NULL) /* 队列是否有效 */
	{ /* 有效         */
		Queue = (DataQueue *)Buf;
		OS_ENTER_CRITICAL();
		Queue->Out = Queue->Buf;
		Queue->In = Queue->Buf;
		Queue->NData = 0; /* 数据数目为0 */
		OS_EXIT_CRITICAL();
	}
}

#endif

/*********************************************************************************************************
 ** 函数名称: QueueNotEnoughPend
 ** 功能描述: 查看队列空间是否足够,若不够,则挂起等待
 ** 输　入: Buf:指向队列的指针;n: 要写入的数据个数
 ** 输　出: 无
 ** 全局变量: 无
 ** 调用模块:
 ** 作　者: Nichenqiang
 ** 日　期: 2010年2月23日
 **-------------------------------------------------------------------------------------------------------
 ** 修改人:
 ** 日　期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
uint8 QueueNotEnoughPend(void* Buf, uint32 n)
{
	uint16 max;
	max = QueueSize(Buf);
	if (n > max) {
		return 0xff; //写数据个数超限
	}
	while ((max - QueueNData((void *) Buf)) < n) {
		OSTimeDly(OS_TICKS_PER_SEC / 10);
	}

	return 0;
}
/*********************************************************************************************************
 **                            End Of File
 ********************************************************************************************************/
