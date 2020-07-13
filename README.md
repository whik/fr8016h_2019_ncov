### 基于FR8016H+ESP8266的新冠肺炎疫情监控平台
----
更好的阅读体验：

- [FR8016H程序运行流程、任务、定时器、串口的使用](http://www.wangchaochao.top/2020/07/12/Program-running-process-task-timer-serial-port-use/)
- [基于FR8016H+ESP8266的新冠肺炎疫情监控平台](http://www.wangchaochao.top/2020/07/12/Novel-coronavirus-pneumonia-surveillance-platform-based-on-FR8016H-ESP8266/)

#### 0.前言

前几天社区管理员在评测群里说，周末截止提交作品。所以我不得不赶紧趁着周末两天的时间，做出一个小设计出来。板子到手也有近一个月的时间了，期间断断续续也试图实现一些功能，但是示例代码实在是看不明白，所以也就没有怎么上心。

果然，人都是有潜力的，压力就是动力，周末这两天，仔细阅读了SDK使用手册和示例代码，也看了不少社区网友分享的经验总结帖子，总算是做出了一个能拿得出手的小设计：**基于FR8016H和ESP8266的新冠肺炎疫情监控平台**。

整体效果：

![fr8016h_2019_ncov_5](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200712/fr8016h_2019_ncov_5.jpeg)

运行界面：

![fr8016h_2019_ncov_6](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200712/fr8016h_2019_ncov_6.jpg)

可以看到，整个设计不需要太多的硬件模块，所需要的只是一个非常常见的ESP8266-01S WiFi模块，还有几个跳线帽。屏幕大小虽然仅有1.54寸，分辨率却有240*240，显示效果非常细腻，所以界面看起来还是非常精致的。

整体设计流程为，FR8016H通过串口与ESP8266模块进行交互，配置模块工作模式，WiFi信息，API接口信息。连接上互联网，获取到API接口返回的最新的全球疫情数据，本地接收完成之后，使用cJSON解析库解析出我们想要的数据，再通过LCD界面显示出来。使用定时器实现每隔一定时间自动获取最新数据，刷新显示。通过读取按键输入，可以手动更新最新的疫情数据。

#### 1.获取疫情API接口

不知道大家是否了解过我之前的几个小项目：

- [基于桌面版Qt的疫情监控平台](https://mbb.eet-china.com/forum/topic/76256_1_1.html)
- [基于嵌入式Qt的疫情监控平台](https://mbb.eet-china.com/forum/topic/78078_1_1.html)

以上两个项目都是基于Qt跨平台环境实现的，所以本质上没有太大的区别。

2020新冠疫情的爆发，各大互联网IT公司和个人都开发了实时疫情地图平台，腾讯新闻、丁香园、网易、新浪等等，这些数据大小都在几百KB，对于桌面PC和嵌入式Linux来说，不用在意数据量的大小，但是对于MCU这种存储只有几百KB的芯片来说，数据的长度就不得不考虑了。更重要的对于ESP8266，AT指令的方式，SSL缓存最大只有4096个字节的缓存！

所以API的选择至关重要，不仅要数据内容齐全，还要数据量小，由于我们要展示的信息比较少，所以我还是找到了一个数据量小，连接稳定的API接口。

经过网上一番搜索，找到了几个数据量小的API，但是有的接口连接不稳定，刚连上就掉线了，最后终于找到了一个连接稳定，数据量小，数据齐全的接口：`https://lab.isaaclin.cn/nCoV/zh`

这是一位国人使用服务器爬虫的方式获取丁香园的数据，然后开发了API接口供大家免费使用，目前已经被调用了几千万次，这个网站还包括了多个接口，我只使用到了其中的疫情数据这一个接口：`https://lab.isaaclin.cn/nCoV/api/overall`，数据量大概为1300个字节。

JSON数据内容如下：

![json数据格式](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200626/2020-06-27_121811.png)

为了能使用ESP8266获取这个API返回的内容，我们还需要知道以下信息：TCP连接类型，端口号，API地址。

我们在浏览器中按F12，打开开发者模式，在地址栏输入`https://lab.isaaclin.cn/nCoV/api/overall`这个接口地址，可以很容易的获取到我们想要的信息：

	服务器地址：47.102.117.253
	端口号：443
	API地址：https://lab.isaaclin.cn/nCoV/api/overall

关于端口号，如果API地址是http开头的，一般是选择TCP连接类型，80端口；如果是https开头的，一般是选择SSL连接类型，443端口。这个信息在后面会用到。

![API获取](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200626/2020-06-27_113602.png)

#### 2.ESP8266的使用

WiFi模块选择的是乐鑫的ESP8266-01S模组，支持AP、Station和AP&Station混合模式。

![ESP8266-01S](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200626/ESP8266-01S.png)

在进行正式的开发之前，我们先使用串口模块连接ESP8266，直接发送AT指令的方式来获取疫情数据。

整体流程是：`配置工作模式 > 连接WiFi > 与服务器建立SSL连接 > 发送GET请求获取数据`

0.为了确保模块保持初始状态，在进行配置之前，先让模块恢复出厂设置：`AT+RESTORE`

```C

AT+RESTORE
    
 ets Jan  8 2013,rst cause:2, boot mode:(3,7)

2nd boot version : 1.5
  SPI Speed      : 40MHz
  SPI Mode       : DIO
  SPI Flash Size & Map: 8Mbit(512KB+512KB)
jump to run user1 @ 1000

ready

```

获取AT固件版本信息：`AT+GMR`

```c

AT+GMR

AT version:1.2.0.0(Jul  1 2016 20:04:45)
SDK version:1.5.4.1(39cb9a32)
Ai-Thinker Technology Co. Ltd.
Dec  2 2016 14:21:16
OK

```

有的AT固件版本不支持HTTPS连接。最新版本的AT固件是支持HTTPS连接的，下载地址：[ESP8266-01S出厂默认 AT 固件](https://docs.ai-thinker.com/_media/esp8266/ai-thinker_esp8266_at_firmware_dout_v1.5.4.1-a_20171130.rar)

1.WiFi模块设置为Station模式：`AT+CWMODE=1`

2.配网，连接WiFi：`AT+CWJAP="ssid","password"`

```c

AT+CWMODE=1

OK
AT+CWJAP="fr8016h_2019_ncov","www.wangchaochao.top"

WIFI CONNECTED
WIFI GOT IP

OK

```

3.设置单连接模式：`AT+CIPMUX=0`

4.设置SSL连接大小：`AT+CIPSSLSIZE=4096`

5.与服务器建立HTTPS/SSL连接：`AT+CIPSTART="SSL","47.102.117.253",443`

6.设置为透传模式：`AT+CIPMODE=1`

7.启动透传：`AT+CIPSEND`

8.发送GET HTTPS请求：`GET https://lab.isaaclin.cn/nCoV/api/overall`

如果以上都配置正确，会收到服务器返回的数据，也就是我们的想要的疫情数据。

如果SSL连接不断开，一直在透传模式，就可以每隔一段时间GET一次API，这样就可以获取到最新的疫情数据了。

经过多次GET请求测试发现，连接还比较稳定，没有出现掉线的情况，但是由于API的访问限制，不要太频繁的发送GET请求，否则可能会被API开发者把IP封掉。

当然，如果连接断开，就要重新执行建立SSL连接，设置透传模式，开始透传这几个操作。如果要主动断开SSL连接，可以先发送不带回车换行的`+++`退出透传，然后使用`AT+CIPCLOSE`关闭SSL连接。

单独的AT指令测试没问题，那我们就可以使用MCU的串口来自动完成和ESP8266的AT指令交互了。 

#### 3.FR8106H串口的使用

FR8016H的每个GPIO使用非常灵活，支持多种复用功能，由于UART1已经使用作为程序下载和调试接口，这里我们使用UART0和ESP8266进行AT指令交互。 

串口0和定时器的初始化：

```c

void app_esp8266_init(void)
{
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTD5_FUNC_UART0_TXD);
    uart_init(UART0, BAUD_RATE_115200);  
    NVIC_EnableIRQ(UART0_IRQn);   

    //10ms中断
    timer_init(TIMER0, TIME, TIMER_PERIODIC); 
    timer_stop(TIMER0);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

```

PD4/5复用成串口功能，并使能中断，使用定时器0定时10ms来判断是不是连续的一帧数据。如果2个字符接收间隔超过10ms，则认为不是1次连续数据，也就是超过10ms没有接收到任何数据，则表示此次接收完毕。

串口中断复位函数：

```c

uint8_t rx_buf[RX_BUF_SIZE];
uint16_t rx_sta = 0;

uint32_t TIME = 10000;

__attribute__((weak)) __attribute__((section("ram_code"))) void uart0_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;
    int_id = uart_reg->u3.iir.int_id;
    if(int_id == 0x04 || int_id == 0x0c )   
    {
        c = uart_reg->u1.data;
        
        if((rx_sta & (1 << 15)) == 0) 
        {
            if(rx_sta < RX_BUF_SIZE)	
            {
                if(rx_sta == 0) 				
                {
                    timer_init(TIMER0, TIME, TIMER_PERIODIC);
                    timer_run(TIMER0);
                }
                rx_buf[rx_sta++] = c;	
            }
            else
            {
                rx_sta |= 1 << 15;	
            }
        }
    }
    else if(int_id == 0x06)
    {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

```

定时器中断函数：

```c

__attribute__((weak)) __attribute__((section("ram_code"))) void timer0_isr_ram(void)
{
    rx_sta |= 1<<15;	//标记接收完成
    timer_stop(TIMER0);
    timer_clear_interrupt(TIMER0);
}

```

ESP8266的串口指令交互，采用发送命令，并检测返回数据的方式，如发送`AT`，应该回复`OK`，如果没有回复，则再次发送`AT`。具体的实现在工程`app_esp8266_wifista.c`文件中

为了方便进行串口发送，这里我自定义实现了串口printf函数：

串口1用于输出调试信息：

```c

void LOG(char *fmt,...)
{
	unsigned char UsartPrintfBuf[296];
	va_list ap;
	unsigned char *pStr = UsartPrintfBuf;
	
	va_start(ap, fmt);
	vsnprintf((char *)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//格式化
	va_end(ap);
	
	while(*pStr != 0)
		uart_putc_noint(UART1, *pStr++);
}

```

串口0用于和ESP8266进行交互：

```c

void esp8266_printf(char *fmt,...)
{
	unsigned char UsartPrintfBuf[296];
	va_list ap;
	unsigned char *pStr = UsartPrintfBuf;
	
	va_start(ap, fmt);
	vsnprintf((char *)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//格式化
	va_end(ap);
	
	while(*pStr != 0)
		uart_putc_noint(UART0, *pStr++);
}

```

实际的流程：

![2020-07-12_223307](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200712/2020-07-12_223307.jpg)

#### 4.疫情数据的解析

API接口返回的数据是JSON格式，关于JSON格式，可以参考：[使用cJSON库解析和构建JSON字符串](https://mp.weixin.qq.com/s/_rS_uh1KIrwqqIfRsdSe5A)

解析库使用的是开源小巧的cJSON库，只有两个文件，使用起来非常方便。由于cJSON采用的动态内存分配的方式，所以在使用之前，要先指定内存申请和内存释放的函数，FR8016H相关的函数在`os_mem.h`头文件中：

- 内存申请：`os_malloc`
- 内存释放：`os_free`

在进行解析之前，先来分析一下JSON原始数据的格式：`results`键的值是一个数组，数组只有一个JSON对象，获取这个对象对应键的值可以获取到国内现存和新增确诊人数、累计和新增死亡人数，累计和新增治愈人数等数据。

全球疫情数据保存在`globalStatistics`键里，它的值是一个JSON对象，对象仅包含简单的键值对，这些键的值，就是全球疫情数据，其中`updateTime`键的值是更新时间，这是毫秒级UNIX时间戳，可以转换为标准北京时间。

```json

{
	"results": [{
		"currentConfirmedCount": 509,
		"currentConfirmedIncr": 16,
		"confirmedCount": 85172,
		"confirmedIncr": 24,
		"suspectedCount": 1899,
		"suspectedIncr": 4,
		"curedCount": 80015,
		"curedIncr": 8,
		"deadCount": 4648,
		"deadIncr": 0,
		"seriousCount": 106,
		"seriousIncr": 9,
		"globalStatistics": {
			"currentConfirmedCount": 4589839,
			"confirmedCount": 9746927,
			"curedCount": 4663778,
			"deadCount": 493310,
			"currentConfirmedIncr": 281,
			"confirmedIncr": 711,
			"curedIncr": 424,
			"deadIncr": 6
		},
		"updateTime": 1593227489355
	}],
	"success": true
}

```

我们先定义了一个结构体，用于存放解析出来的数据：

```c

struct ncov_data{
    long currentConfirmedCount;
    long currentConfirmedIncr;
    long confirmedCount;
    long confirmedIncr;
    long curedCount;
    long curedIncr;
    long seriousCount;
    long seriousIncr;
    long deadCount;
    long deadIncr;
    char updateTime[20];
};

```

由于全球疫情数据，都已经到了千万级别，而且新增数据有负数，即减少情况，所以这里数据类型统一选择了有符号的长整形。

定义了两个结构体，用于保存国内和全球疫情数据：

```c

struct ncov_data dataChina = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "01-01 01:00"};
struct ncov_data dataGlobal = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "01-01 01:00"};

```

完整的解析函数：

```c

uint8_t parse_ncov_data(void)
{
    cJSON *root;
    cJSON *results_arr;
    cJSON *results;
    cJSON *globalStatistics;
    time_t updateTime;
    struct tm *time;

    LOG("接收到的数据:%d\r\n", strlen((const char *)rx_buf));	//JSON原始数据
    root = cJSON_Parse((const char *)rx_buf);
    if(root)
    {
        LOG("数据格式正确，开始解析\r\n");

        results_arr = cJSON_GetObjectItem(root, "results");
        if(results_arr->type == cJSON_Array)
        {
            results = cJSON_GetArrayItem(results_arr, 0);
            if(results->type == cJSON_Object)
            {
                dataChina.currentConfirmedCount = cJSON_GetObjectItem(results, "currentConfirmedCount")->valueint;
                dataChina.currentConfirmedIncr  = cJSON_GetObjectItem(results, "currentConfirmedIncr")->valueint;
                dataChina.confirmedCount        = cJSON_GetObjectItem(results, "confirmedCount")->valueint;
                dataChina.confirmedIncr         = cJSON_GetObjectItem(results, "confirmedIncr")->valueint;
                dataChina.curedCount            = cJSON_GetObjectItem(results, "curedCount")->valueint;
                dataChina.curedIncr             = cJSON_GetObjectItem(results, "curedIncr")->valueint;
                dataChina.deadCount             = cJSON_GetObjectItem(results, "deadCount")->valueint;
                dataChina.deadIncr              = cJSON_GetObjectItem(results, "deadIncr")->valueint;
                dataChina.seriousCount          = cJSON_GetObjectItem(results, "seriousCount")->valueint;
                dataChina.seriousIncr           = cJSON_GetObjectItem(results, "seriousIncr")->valueint;

                LOG("------------国内疫情-------------\r\n");
                LOG("现存确诊:   %5d, 较昨日:%3d\r\n", dataChina.currentConfirmedCount, dataChina.currentConfirmedIncr);
                LOG("累计确诊:   %5d, 较昨日:%3d\r\n", dataChina.confirmedCount, dataChina.confirmedIncr);
                LOG("累计治愈:   %5d, 较昨日:%3d\r\n", dataChina.curedCount, dataChina.curedIncr);
                LOG("累计死亡:   %5d, 较昨日:%3d\r\n", dataChina.deadCount, dataChina.deadIncr);
                LOG("现存无症状: %5d, 较昨日:%3d\r\n\r\n", dataChina.seriousCount, dataChina.seriousIncr);

                globalStatistics = cJSON_GetObjectItem(results, "globalStatistics");
                if(globalStatistics->type == cJSON_Object)
                {
                    dataGlobal.currentConfirmedCount = cJSON_GetObjectItem(globalStatistics, "currentConfirmedCount")->valueint;
                    dataGlobal.confirmedCount        = cJSON_GetObjectItem(globalStatistics, "confirmedCount")->valueint;
                    dataGlobal.curedCount            = cJSON_GetObjectItem(globalStatistics, "curedCount")->valueint;
                    dataGlobal.deadCount             = cJSON_GetObjectItem(globalStatistics, "deadCount")->valueint;
                    dataGlobal.currentConfirmedIncr  = cJSON_GetObjectItem(globalStatistics, "currentConfirmedIncr")->valueint;
                    dataGlobal.confirmedIncr         = cJSON_GetObjectItem(globalStatistics, "confirmedIncr")->valueint;
                    dataGlobal.curedIncr             = cJSON_GetObjectItem(globalStatistics, "curedIncr")->valueint;
                    dataGlobal.deadIncr              = cJSON_GetObjectItem(globalStatistics, "deadIncr")->valueint;

                    LOG("------------全球疫情-------------\r\n");
                    LOG("现存确诊: %8d, 较昨日:%5d\r\n", dataGlobal.currentConfirmedCount, dataGlobal.currentConfirmedIncr);
                    LOG("累计确诊: %8d, 较昨日:%5d\r\n", dataGlobal.confirmedCount, dataGlobal.confirmedIncr);
                    LOG("累计死亡: %8d, 较昨日:%5d\r\n", dataGlobal.deadCount, dataGlobal.deadIncr);
                    LOG("累计治愈: %8d, 较昨日:%5d\r\n\r\n", dataGlobal.curedCount, dataGlobal.curedIncr);
                }

                /* 毫秒级时间戳转字符串 */
                updateTime = (time_t )(cJSON_GetObjectItem(results, "updateTime")->valuedouble / 1000);
                updateTime += 8 * 60 * 60; /* UTC8校正 */
                time = localtime(&updateTime);
                /* 格式化时间 */
                strftime(dataChina.updateTime, 20, "%m-%d %H:%M", time);
                LOG("更新于:%s\r\n", dataChina.updateTime);/* 06-24 11:21 */
            }
        }
        else
        {
            LOG("数据格式错误\r\n");
            return 0;
        }
    }

    cJSON_Delete(root);

    LOG("*********更新完成*********\r\n");
    return 1;
}

```

在调用cJSON_Parse()之后，一定要调用cJSON_Delete()释放内存，否则会造成内存泄露。

#### 5.疫情数据的显示

FR8016H开发板板载了一块超薄的1.54寸的LCD，虽然尺寸很小，但是分辨率却有240*240，显示效果非常细腻。

自定义实现了一些GUI绘图的函数，并简单设计了显示界面，为了减小程序大小，LCD驱动只实现了基本的画点，画线函数，字符的显示，采用的是部分字符取模，只对程序中用到的汉子和字符进行取模。具体的代码在工程中的`app_lcd_gui.c`文件。

最终效果：

![fr8016h_2019_ncov_3](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200712/fr8016h_2019_ncov_3.jpeg)

更换了显示颜色：

![fr8016h_2019_ncov_4](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200712/fr8016h_2019_ncov_4.jpeg)

#### 6.自动更新和按键交互

为了让数据能定时自动更新，这里我们使用了软件定时器来实现定时自动更新功能：

定义一个定时器：

```c

os_timer_t timer_ncov;

```

初始化软件定时：

```c

os_timer_init(&timer_ncov, ncov_update, 0);
os_timer_start(&timer_ncov, 1000, true);  //1s

```

数据更新函数：

```c

void ncov_update(void *parg)
{
    static uint16_t cnt = 0;
    int ret;
    
    cnt++;
    LOG("cnt: %d, k1:%d\r\n", cnt, read_btn_k1());
    if(cnt >= 60*5)
    {
        cnt = 0;
        ret = get_ncov_api(api_data, parse_ncov_data);
        if(ret == 1)
        {
            ret = build_ssl_connect(data_cip_type, data_api_ip, data_api_port);
            if(ret == 1)
                app_esp8266_wifista_config(WIFI_SSID, WIFI_PWD);
        }
    }
}

```

由于没看明白示例工程中的Button读取方式，所以这里我直接使用了一个开源的MultiButton按键驱动库，

配置K1/PC5按键为输入功能：

```c

system_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PORTC5_FUNC_C5);
gpio_set_dir(GPIO_PORT_C, GPIO_BIT_5, GPIO_DIR_IN);
system_set_port_pull(GPIO_PC5, true);

```

读取按键输入状态：

```c

uint8_t read_btn_k1(void)
{
    uint8_t in;
    in = gpio_portc_read() & (1<<5);
    return in>>5;
}

```

定义一个Button，并初始化：

```c

struct Button btn_k1;
mbutton_init(&btn_k1, read_btn_k1, 0);

```

绑定回调函数：

```c

button_attach(&btn_k1, SINGLE_CLICK, button_callback);

```

回调函数的实现：

```c

void button_callback(void *button)
{
    uint32_t btn_event_val; 
    
    btn_event_val = get_button_event((struct Button *)button); 
    
    switch(btn_event_val)
    {
	    case SINGLE_CLICK: 
	        co_printf("---> key1 single click! <---\r\n");
            get_ncov_api(api_data, parse_ncov_data);
	    	break; 
	    case DOUBLE_CLICK: 
	        co_printf("***> key1 double click! <***\r\n");
	    	break; 
        default:break;
	}
}

```

这里实现的是K1单击，更新疫情数据。 

定义一个软件定时器：

```c

os_timer_t timer_k1;

os_timer_init(&timer_k1, timer_k1_fun, 0);
os_timer_start(&timer_k1, 10, true); 

```

定时器的函数：

```c

void timer_k1_fun(void *parg)
{
    button_ticks();
}

```

启动按键：

```c

button_start(&btn_k1);

```

#### 7.代码下载

代码已经开源，欢迎大家参与，丰富这个小项目的功能！

GitHub开源地址：`https://github.com/whik/fr8016h_2019_ncov`

我的开发板版本是V1.3，如果你手上的硬件和我的一样，只需要修改工程中的`user\proj_main.h`文件中的WiFi信息，就可以直接使用了。

工程目录结构：

![2020-07-12_231055](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200712/2020-07-12_231055.jpg)

#### 8.总结

目前只使用到了一个API接口，这个平台也提供其他接口可供使用：

- 最新的疫情新闻

`https://lab.isaaclin.cn//nCoV/api/news`

- 最新的各省市疫情数据

`https://lab.isaaclin.cn//nCoV/api/area?latest=1&province=%E5%8C%97%E4%BA%AC%E5%B8%82`

- 最新的辟谣信息

`https://lab.isaaclin.cn//nCoV/api/rumors`

对于这个疫情监控的小项目，还有很多可以完善的功能，FR8016H芯片还支持音频解码功能，可以实现自动语音播报，UI界面可以增加更多的疫情信息显示，如最新的疫情动态新闻，疫情辟谣信息等等。作为一款蓝牙MCU，以后会学习一下蓝牙部分的开发。

![fr8016h_2019_ncov_1](https://wcc-blog.oss-cn-beijing.aliyuncs.com/img/200712/fr8016h_2019_ncov_2.jpeg)

除此之外，即使疫情结束，有了ESP8266和液晶显示屏，也可以做出很多有意思的项目，通过选择不同的API接口，可以实现不同的功能，如天气台历、每日新闻显示、空气质量、股票信息、汽车限行信息等等。

也可以使用ESP8266连接云平台，如OneNET、阿里云、华为云、腾讯云等物联网云平台，实现智能家居功能，远程监控和远程控制设备。