/*********************************************************************************
 * File Name: creatTestData.cpp 
 * Description: 根据"中国气象站点参数.ini"文件中的全国气象站点，模拟生成气象分钟数据
 * Author: jinglong
 * Date: 2020年3月31日 15:09
 * History: 
 *********************************************************************************/

#include "creatTestData.h"

// 气象站点数据结构
// 比如“安徽,58236,滁州,32.21,118.15,33.5”的含义为: 省份名称 站点代码 城市名称 纬度 经度 海拔高度
struct st_WeatherSiteInfo
{
    char provename[128];        // 省份名称
    char obtid[64];             // 站点代码
    char cityname[128];         // 城市名称
    double lat;                 // 纬度
    double lon;                 // 经度
    double height;              // 海拔高度
};

// 分钟观测数据数据结构
struct st_surfdata
{
    char cObtid[64];             // 站点代码
    char cDatatime[128];         // 数据时间(格式: yyyy-mm-dd hh:mi:ss)
    int iTemperature;            // 温度(0.1摄氏度: 保留一位小数)
    int iAirp;                   // 气压(0.1百帕: 保留一位小数)    
    int ihumidity;               // 相对湿度(0-100之间的值)
    int iWindDirec;              // 风向(0-360之间的值)
    int iWindSpeed;              // 风速(0.1m/s)
    int iRainFall;               // 降雨量(0.1mm)
    int iVisibility;             // 能见度(0.1m)
};

vector<st_WeatherSiteInfo> vstcode;      // 存放全国气象站点参数的容器
vector<st_surfdata> vSimulatedata;       // 存放全国气象站点观测到的分钟数据(模拟值)
extern CLogManager* logFile;

/*******************************************************************************
 * 函数名称: loadSTCode
 * 函数功能: 将"中国气象站点参数.ini"文件中的全国气象站点参数加载到vstcode中
 * 输入参数: const char *iniFilePath    "中国气象站点参数.ini"文件路径
 * 输出参数: 无
 * 返 回 值: true: 成功/false: 失败
 *******************************************************************************/
bool loadSTCode(const char *iniFilePath)
{
    CFile file;
    CStrOperation strOper;
    struct st_WeatherSiteInfo stnode;
    char strBuffer[128] = {'\0'};
    char* result = NULL;

    vstcode.clear();

    if (file.openFile(iniFilePath, "r") == false)
    {
        logFile->WriteLogFile("气象站点参数文件打开失败, 文件路径为%s\n", iniFilePath);
        return false;
    }
    
    while (true)
    {
        if (file.f_fgets(strBuffer, sizeof(strBuffer)) == false)
        {
            logFile->WriteLogFile("end of file!!!\n");
            break;
        }
    
        // 按照逗号拆分字符串：“安徽,58236,滁州,32.21,118.15,33.5”，放到m_vSplitedStr中
        strOper.strSplit(strBuffer, ",");
        
        // 利用拆分后的字符串，填充结构体stnode，并放到vstcode中
        strOper.setWeatherSiteInfo(0, stnode.provename);
        strOper.setWeatherSiteInfo(1, stnode.obtid);
        strOper.setWeatherSiteInfo(2, stnode.cityname);
        strOper.setWeatherSiteInfo(3, &stnode.lat);
        strOper.setWeatherSiteInfo(4, &stnode.lon);
        strOper.setWeatherSiteInfo(5, &stnode.height);
        vstcode.push_back(stnode);
    }

    return true;
}

/*******************************************************************************
 * 函数名称: createSimulateData
 * 函数功能: 生成模拟分钟数据
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 *******************************************************************************/
void createSimulateData()
{
    vSimulatedata.clear();
    struct tm *stDateTime;
    struct st_surfdata simulateData;
    char dateTimeBuf[128] = {'\0'};

    srand(time(0));
    stDateTime = getLocalDateTime();
    sprintf(dateTimeBuf, "%04u-%02u-%02u %2u:%02u:%02u", stDateTime->tm_year + 1900, stDateTime->tm_mon, stDateTime->tm_mday, 
            stDateTime->tm_hour, stDateTime->tm_min, stDateTime->tm_sec);

    for (vector<st_WeatherSiteInfo>::iterator it = vstcode.begin(); it != vstcode.end(); it++)
    {
        strcpy(simulateData.cObtid, it->obtid);
        strcpy(simulateData.cDatatime, dateTimeBuf);
        simulateData.iTemperature = rand() % 351;
        simulateData.iAirp = rand() % 265 + 10000;
        simulateData.ihumidity = rand() % 100 + 1;
        simulateData.iWindDirec = rand() % 360;
        simulateData.iWindSpeed = rand() % 150;
        simulateData.iRainFall = rand() % 16;
        simulateData.iVisibility = rand() % 5001 + 100000;

        vSimulatedata.push_back(simulateData);
    }
}

/*******************************************************************************
 * 函数名称: saveSimulateDataToFile
 * 函数功能: 将模拟分钟数据存入文件
 * 输入参数: const char *surfDataFileDir       模拟数据存放目录
 * 输出参数: 无
 * 返 回 值: true: 成功/false: 失败
 *******************************************************************************/
bool saveSimulateDataToFile(const char *surfDataFileDir)
{
    CFile file;
    char strNewFileName[128] = {'\0'};
    char strFileName[128] = {'\0'};
    struct tm *stDateTime;
    stDateTime = getLocalDateTime();

    sprintf(strFileName, "%s%04u%02u%02u%2u%02u%02u.txt.tmp", surfDataFileDir, stDateTime->tm_year + 1900, stDateTime->tm_mon, stDateTime->tm_mday, 
            stDateTime->tm_hour, stDateTime->tm_min, stDateTime->tm_sec);

    sprintf(strNewFileName, "%s%04u%02u%02u%2u%02u%02u.txt", surfDataFileDir, stDateTime->tm_year + 1900, stDateTime->tm_mon, stDateTime->tm_mday, 
            stDateTime->tm_hour, stDateTime->tm_min, stDateTime->tm_sec);

    if (file.openFile(strFileName, "a") == false)
    {
        logFile->WriteLogFile("打开气象观测分钟数据文件失败, 文件路径为%s\n", strFileName);
        return false;
    }

    for (vector<st_surfdata>::iterator it = vSimulatedata.begin(); it != vSimulatedata.end(); it++)
    {
        // 写入格式：站点代码, 数据时间, 气温, 气压, 相对湿度, 风向, 风速, 降雨量, 能见度
        file.f_fprintf("%s, %s, %.1f%, %.1f, %d, %d, %.1f, %.1f, %.1f\n", it->cObtid, it->cDatatime, 
            it->iTemperature / 10.0, it->iAirp / 10.0, it->ihumidity, it->iWindDirec, it->iWindSpeed / 10.0, it->iRainFall / 10.0, it->iVisibility / 10.0);
    }

    vSimulatedata.clear();
    vstcode.clear();
    file.closeFile();

    if (rename(strFileName, strNewFileName) != 0)
    {
        logFile->WriteLogFile("rename file name error!!!\n");
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    string strIniFilePath;              // 中国气象站点参数文件路径
    string strGeneFilePath;             // 生成的数据文件存放路径
    CXMLConfigManager config("config.xml");

    // 读取配置信息
    if (!config.getValue("Config/createData/inifilepath", strIniFilePath))
    {
        logFile->WriteLogFile("failed to load parameter inifilepath from config file\n");
        return -1;
    }

    if (access(strIniFilePath.c_str(), F_OK) != 0)
    {
        logFile->WriteLogFile("文件不存在，文件路径为%s\n", strIniFilePath.c_str());
        return -1;
    }

    if (!config.getValue("Config/createData/generatefilepath", strGeneFilePath))
    {
        logFile->WriteLogFile("failed to load parameter generatefilepath from config file\n");
        return -1;
    }

    if(access(strGeneFilePath.c_str(), F_OK) != 0)
    {
        logFile->WriteLogFile("文件不存在，文件路径为%s\n", strIniFilePath.c_str());
        return -1;
    }

    // 将全国所有的气象站点参数加载到vector中
    if (loadSTCode(strIniFilePath.c_str()) == false)
    {
        logFile->WriteLogFile("加载气象站点参数文件失败，文件路径为%s\n", strIniFilePath.c_str());
        return -1;
    }

    logFile->WriteLogFile("加载气象站点参数文件成功\n");

    // 生成模拟数据，放到vsurfdata容器中
    createSimulateData();
    
    // 将模拟数据写入到文件中
    if (false == saveSimulateDataToFile(strGeneFilePath.c_str()))
    {
        logFile->WriteLogFile("生成模拟气象观测分钟数据失败\n");
    }
    else 
    {
        logFile->WriteLogFile("生成模拟分钟数据成功\n");
    }

    return 0;
}


