/*
 * File:   uconsole.c
 * Author: Spas Spasov
 *
 * Created on May 2019
 */

#include <xc.h>
#include "plibs/plib.h"
#include "uconsole.h"


// *****************************************************************************
/* System Objects

  Summary:
    Structure holding the system's object handles

  Description:
    This structure contains the object handles for all objects in the
    MPLAB Harmony project's system configuration.

  Remarks:
    These handles are returned from the "Initialize" functions for each module
    and must be passed into the "Tasks" function for each module.
*/
typedef struct
{
    SYS_MODULE_OBJ  sysDevcon;
    SYS_MODULE_OBJ  sysTmr;
    SYS_MODULE_OBJ  drvTmr0;
    SYS_MODULE_OBJ  drvUSBObject;

    SYS_MODULE_OBJ  usbDevObject0;



} SYSTEM_OBJECTS;


// *****************************************************************************
/* Application States

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
  /* Application opens and attaches the device here */
    APP_STATE_INIT = 0,

    /* Application waits for device configuration*/
    APP_STATE_WAIT_FOR_CONFIGURATION,

    /* Check if we got data from CDC */
    APP_STATE_CHECK_CDC_READ_WRITE,

    /* Application Error state*/
    APP_STATE_ERROR

} APP_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* Device layer handle returned by device layer open function */
    USB_DEVICE_HANDLE deviceHandle;

    /* Application CDC Instance */
    USB_DEVICE_CDC_INDEX cdcInstance;

    /* Application USART Driver handle */
    DRV_HANDLE usartHandle;

    /* Application's current state*/
    APP_STATES state;

    /* Device configured state */
    bool isConfigured;

    /* Flag indicates that SOF event has occurred */
    bool sofEventHasOccurred;

    /* Read Data Buffer */
    uint8_t * readBuffer;

    /* Set Line Coding Data */
    USB_CDC_LINE_CODING setLineCodingData;

    /* Get Line Coding Data */
    USB_CDC_LINE_CODING getLineCodingData;

    /* Control Line State */
    USB_CDC_CONTROL_LINE_STATE controlLineStateData;

    /* Break data */
    uint16_t breakData;

    /* Read transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE readTransferHandle;

    /* Write transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE writeTransferHandle;

    /* total files in the volume */
    uint32_t totalFiles;

    /* True if a character was read */
    bool isReadComplete;

    /* True if a character was written*/
    bool isWriteComplete;

    /* UART2 received data */
    uint8_t * uartReceivedData;

    /* Read Buffer Length*/
    size_t readLength;

    /* Current UART TX Count*/
    size_t uartTxCount;


} APP_DATA;


/*******************************************************************************
 * Global Data Definitions:
 ******************************************************************************/
// PIC32MX U1OTGSTAT emulation (only for VBUSVD bit).
#define U1OTGSTAT                       ((USBOTGbits.VBUS == 3) ? 1 : 0)

#define SYS_DEVCON_INDEX_0              0

/******************************************************
 * USB Driver Initialization
 ******************************************************/
const DRV_USBHS_INIT drvUSBInit =
{
    /* Interrupt Source for USB module */
    .interruptSource = INT_SOURCE_USB_1,

    /* Interrupt Source for USB module */
    .interruptSourceUSBDma = INT_SOURCE_USB_1_DMA,

    /* System module initialization */
    .moduleInit = {SYS_MODULE_POWER_RUN_FULL},

    .operationMode = DRV_USBHS_OPMODE_DEVICE,

    .operationSpeed = USB_SPEED_HIGH,

    /* Stop in idle */
    .stopInIdle = false,

    /* Suspend in sleep */
    .suspendInSleep = false,

    /* Identifies peripheral (PLIB-level) ID */
    .usbID = USBHS_ID_0,
};


APP_DATA appData;

/* Structure to hold the object handles for the modules in the system. */
SYSTEM_OBJECTS sysObj;

extern const USB_DEVICE_INIT usbDevInitData;

static BOOL checkIsUSBInitialized = FALSE;

// USB buffers
char __attribute__((coherent)) __attribute__((aligned(16))) USB_RxBuf[2][USB_RX_BUFFER_SIZE];
char __attribute__((coherent)) __attribute__((aligned(16))) USB_TxOutBuf[USB_TX_BUFFER_SIZE];
char USB_TxBuf[USB_TX_BUFFER_SIZE];
volatile int USBTxBufHead = 0;
volatile int USBTxBufTail = 0;
volatile int USB_NbrCharsInTxBuf;
volatile int USB_Current_RxBuf;
volatile int USBTxBufCount = 0;

// Because the USB copies data direct to the serial Tx buf and sends data direct from the serial Rx queue
// it does not need its own I/O buffers (except for the hardware device buffers declared above).
// The only pointer that we need is this one which keep track of where we are while reading the serial Rx buffer
//volatile int USBSerialRxBufTail = 0;
//char xxConsoleRxBuf[CONSOLE_RX_BUF_SIZE];
char ConsoleRxBuf[CONSOLE_RX_BUF_SIZE];
volatile int ConsoleRxBufHead = 0;
volatile int ConsoleRxBufTail = 0;

char ConsoleTxBuf[CONSOLE_TX_BUF_SIZE];
volatile int ConsoleTxBufHead = 0;
volatile int ConsoleTxBufTail = 0;
volatile int ConsoleTxBufCount = 0;

/** PRIVATE FUNCTION PROTOTYPES ************************************************/
void USBAppInitialize (void);
bool APP_StateReset(void);
void APP_USBDeviceEventHandler ( USB_DEVICE_EVENT event, void * eventData, uintptr_t context);
USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler(USB_DEVICE_CDC_INDEX index ,
                                                            USB_DEVICE_CDC_EVENT event ,
                                                            void* pData,
                                                            uintptr_t userData);


/*********************************************************************************************
* USB Serial I/O functions for the console
**********************************************************************************************/

// initialize the USB serial console and USB
void initUSBConsole(void)
{
uint32_t processorStatus;

    processorStatus = INTDisableInterrupts();

    if(checkIsUSBInitialized == FALSE) {
        ConsoleRxBufHead = ConsoleRxBufTail = 0;
        USB_NbrCharsInTxBuf = 0;
        USB_Current_RxBuf = 0;
        USBTxBufCount = 0;
        ConsoleTxBufCount = 0;

        /* Initialize Drivers */

        sysObj.sysDevcon = SYS_DEVCON_INDEX_0;

        /* Initialize USB Driver */
        sysObj.drvUSBObject = DRV_USBHS_Initialize(DRV_USBHS_INDEX_0, (SYS_MODULE_INIT *) &drvUSBInit);

        /* Set priority of USB interrupt source */
        INTSetVectorPriority(INT_VECTOR_USB1, INT_PRIORITY_LEVEL4);

        /* Set Sub-priority of USB interrupt source */
        INTSetVectorSubPriority(INT_VECTOR_USB1, INT_SUBPRIORITY_LEVEL0);

        /* Set the priority of the USB DMA Interrupt */
        INTSetVectorPriority(INT_VECTOR_USB1_DMA, INT_PRIORITY_LEVEL4);

        /* Set Sub-priority of the USB DMA Interrupt */
        INTSetVectorSubPriority(INT_VECTOR_USB1_DMA, INT_SUBPRIORITY_LEVEL0);

        /* Initialize the USB device layer */
        sysObj.usbDevObject0 = USB_DEVICE_Initialize (USB_DEVICE_INDEX_0 , ( SYS_MODULE_INIT* ) & usbDevInitData);

        mSysUnlockOpLock((CFGCON &= 0xFFFFFEFF));

        /* Initialize the Application */
        USBAppInitialize();

        checkIsUSBInitialized = TRUE;
    }
    else {
        IEC4bits.USBIE = 1;
        IEC4bits.USBDMAIE = 1;
        #ifdef  USE_UTIMER_USBDEVICE_SYS_TASK
        /* start the Interrupt of the USB UTimer device task" */
        USB_Device_StartTaskTimer(USB_DEVICE_UTIMER, USB_DEVICE_UTIMER_TICK);
        #endif
    }

    INTRestoreInterrupts(processorStatus);
}

// close the USB serial console and USB
void CloseUSBConsole(void)
{
uint32_t processorStatus;

    processorStatus = INTDisableInterrupts();

    IEC4bits.USBDMAIE = 0;
    IEC4bits.USBIE = 0;
    #ifdef  USE_UTIMER_USBDEVICE_SYS_TASK
    /* stop the Interrupt of the USB UTimer device task" */
    USB_Device_StopTaskTimer(USB_DEVICE_UTIMER);
    #endif
    mSysUnlockOpLock((CFGCONbits.USBSSEN = 0));

    INTRestoreInterrupts(processorStatus);
}


/*********************************************************************************************
* Serial I/O functions for the console
**********************************************************************************************/

// send a character to the USB serial port
void SerUSBPutC(int c)
{

    if(U1OTGSTAT & 1)
    {
		USB_TxBuf[USBTxBufHead] = c;							// add the char
		USBTxBufHead = (USBTxBufHead + 1) % USB_TX_BUFFER_SIZE;		   // advance the head of the queue
        USBTxBufCount++;
        if(USBTxBufHead==USBTxBufTail) {USBTxBufTail = (USBTxBufTail + 1) % USB_TX_BUFFER_SIZE; USBTxBufCount--;} //through away the oldest character if buffer full
    }
}


// print a string on the Serial and USB consoles only (used in the EDIT command and dp() macro)
void SerUSBPutS(char *s)
{

    while(*s) SerUSBPutC(*s++);
}


// get a char from the console input queue
// will return immediately with -1 if there is no character waiting
int serUSBGetC(void)
{
int c;

    if(ConsoleRxBufHead == ConsoleRxBufTail) return -1;
    c = ConsoleRxBuf[ConsoleRxBufTail];
    ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;    // advance the head of the queue
    return c;
}



/*********************************************************************************************
* USB I/O functions
**********************************************************************************************/

/******************************************************************************************
Check the USB for work to be done.
Used to send and get data to or from the USB interface.
Each call takes typically 6uS but sometimes it can be up to 600uS.
*******************************************************************************************/
void __attribute__((optimize("-O0"))) CheckUSB(void)
{
size_t i, numBytesRead;

    /* Update the application state machine based
     * on the current state */
    switch(appData.state)
    {
        case APP_STATE_INIT:

            /* Open the device layer */
            appData.deviceHandle = USB_DEVICE_Open( USB_DEVICE_INDEX_0,    DRV_IO_INTENT_READWRITE );

            if(appData.deviceHandle != USB_DEVICE_HANDLE_INVALID)
            {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(appData.deviceHandle, APP_USBDeviceEventHandler, 0);

                /* Application waits for device configuration. */
                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
            }
            else
            {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }

            break;

        case APP_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device was configured */
            if(appData.isConfigured)
            {
                /* Schedule the first read from CDC function driver */

                appData.state = APP_STATE_CHECK_CDC_READ_WRITE;
                appData.isReadComplete = false;
                USB_DEVICE_CDC_Read (appData.cdcInstance, &(appData.readTransferHandle),
                                        appData.readBuffer, APP_READ_BUFFER_SIZE);
            }
            break;

        case APP_STATE_CHECK_CDC_READ_WRITE:
            if(APP_StateReset())
            {
                break;
            }

            /* If CDC read is complete ... */
            if(appData.isReadComplete == true)
            {
                appData.readBuffer = USB_RxBuf[(!USB_Current_RxBuf)];
                numBytesRead = appData.readLength;
                appData.isReadComplete = false;
                appData.readTransferHandle =  USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
                USB_DEVICE_CDC_Read (appData.cdcInstance, &appData.readTransferHandle,
                                        appData.readBuffer, APP_READ_BUFFER_SIZE);
                if(numBytesRead > 0)
                {
                    for(i = 0; i < numBytesRead; i++) {                                         // if we have some data, copy it into the keyboard buffer
                        ConsoleRxBuf[ConsoleRxBufHead] = USB_RxBuf[USB_Current_RxBuf][i];       // store the byte in the ring buffer

                        ConsoleRxBufHead = (ConsoleRxBufHead + 1) % CONSOLE_RX_BUF_SIZE;        // advance the head of the queue
                        if(ConsoleRxBufHead == ConsoleRxBufTail) {                              // if the buffer has overflowed
                            ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE;    // throw away the oldest char
                        }
                    }
                }
                USB_Current_RxBuf = !USB_Current_RxBuf;
            }

            /* Next, check for data to be sent. */
            if(USBTxBufHead != USBTxBufTail && appData.isWriteComplete) {
                USB_NbrCharsInTxBuf = 0;
                if( (ConsoleTxBufCount == 0) || (USBTxBufCount <= ConsoleTxBufCount) || ((USBTxBufCount > ConsoleTxBufCount) && ((USBTxBufCount - ConsoleTxBufCount) < 60)) )
                {
                    while((USBTxBufHead != USBTxBufTail) && (USB_NbrCharsInTxBuf < 512)) {
                        USB_TxOutBuf[USB_NbrCharsInTxBuf++] = USB_TxBuf[USBTxBufTail];
                        USBTxBufTail = (USBTxBufTail + 1) % USB_TX_BUFFER_SIZE;                 // advance the tail of the queue
                        USBTxBufCount--;
                    }
                }
                else if(USBTxBufCount > 0) {

                    USB_TxOutBuf[USB_NbrCharsInTxBuf++] = USB_TxBuf[USBTxBufTail];
                    USBTxBufTail = (USBTxBufTail + 1) % USB_TX_BUFFER_SIZE;                     // advance the tail of the queue
                    USBTxBufCount--;
                }
                else
                {
                    break;
                }
                appData.isWriteComplete = false;
                appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
                USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0, &appData.writeTransferHandle,
                                        USB_TxOutBuf, USB_NbrCharsInTxBuf,
                                        USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
            }

            break;

        case APP_STATE_ERROR:
        default:
            appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
            appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
             appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
            appData.isReadComplete = true;
            appData.isWriteComplete = true;
            break;
    }
}


/*******************************************************************************
 * Section: Application Local Functions:
 ******************************************************************************/

/*****************************************************
 * This function is called in every step of the
 * application state machine.
 *****************************************************/
bool APP_StateReset(void)
{
    /* This function returns true if the device
     * was reset  */

    bool retVal;

    if(appData.isConfigured == false)
    {
        appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
        appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appData.isReadComplete = true;
        appData.isWriteComplete = true;

        retVal = true;
    }
    else
    {
        retVal = false;
    }

    return(retVal);
}


/*******************************************************************************
 *  Application Initialization and State Machine Functions:
 ******************************************************************************/

/*******************************************************************************
 *Function:
 *  void APP_Initialize ( void )
 ******************************************************************************/
void USBAppInitialize ( void )
{
     /* Device Layer Handle  */
    appData.deviceHandle = USB_DEVICE_HANDLE_INVALID;

    /* USART Driver Handle */
    appData.usartHandle = DRV_HANDLE_INVALID;

    /* CDC Instance index for this app object00*/
    appData.cdcInstance = USB_DEVICE_CDC_INDEX_0;

    /* app state */
    appData.state = APP_STATE_INIT ;

    /* device configured status */
    appData.isConfigured = false;

    /* Initial get line coding state */
    appData.getLineCodingData.dwDTERate = 115200;
    appData.getLineCodingData.bDataBits = 8;
    appData.getLineCodingData.bParityType = 0;
    appData.getLineCodingData.bCharFormat = 0;

    /* Read Transfer Handle */
    appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Write Transfer Handle */
    appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Intialize the read complete flag */
    appData.isReadComplete = true;

    /*Initialize the write complete flag*/
    appData.isWriteComplete = true;

    /*Initialize the buffer pointers */
    appData.readBuffer = USB_RxBuf[0];

    appData.uartTxCount = 0;

    appData.totalFiles = 0;
}


/*******************************************************************************
 * Application Callback Functions:
 ******************************************************************************/

/*******************************************************
 * USB CDC Device Events - Application Event Handler
 *******************************************************/
USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler
(
    USB_DEVICE_CDC_INDEX index ,
    USB_DEVICE_CDC_EVENT event ,
    void* pData,
    uintptr_t userData
)
{
    APP_DATA * appDataObject;
    appDataObject = (APP_DATA *)userData;
    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;
    uint16_t * breakData;

    switch ( event )
    {
        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:
            /* This means that the host has sent some data*/
            appDataObject->readLength =
                    ((USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE*)pData)->length;
            appDataObject->isReadComplete = true;
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:
            /* The data stage of the last control transfer is
             * complete. We have received the line coding from the host. Call
             * the usart driver routine to change the baud. We can call this
             * function here as it is not blocking. */
            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:
            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  */
             USB_DEVICE_ControlSend(appDataObject->deviceHandle,
                    &appDataObject->getLineCodingData, sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:
            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host */
            USB_DEVICE_ControlReceive(appDataObject->deviceHandle,
                    &appDataObject->setLineCodingData, sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:
            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. */
            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *)pData;
            appDataObject->controlLineStateData.dtr = controlLineStateData->dtr;
            appDataObject->controlLineStateData.carrier =
                    controlLineStateData->carrier;

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:
            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration */
            breakData = (uint16_t *)pData;
            appDataObject->breakData = *breakData;

            /* Complete the control transfer by sending a ZLP  */
            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:
            /* This means the GET LINE CODING function data is valid. We dont
             * do much with this data in this demo. */
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:
            /* This means that the data write got completed. We can schedule
             * the next read. */
            appDataObject->isWriteComplete = true;
            break;

        default:
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}


/*******************************************************
 * USB Device Layer Events - Application Event Handler
 *******************************************************/
void APP_USBDeviceEventHandler ( USB_DEVICE_EVENT event, void * eventData, uintptr_t context)
{
    uint8_t configurationValue;
    switch ( event )
    {
        case USB_DEVICE_EVENT_RESET:
        case USB_DEVICE_EVENT_DECONFIGURED:
            /* USB device is reset or device is deconfigured.
             * This means that USB device layer is about to deininitialize
             * all function drivers. */
            appData.isConfigured = false;
            break;

        case USB_DEVICE_EVENT_SOF:

            /* This event is used for switch debounce. This flag is reset
             * by the switch process routine. */
            appData.sofEventHasOccurred = true;
            break;

        case USB_DEVICE_EVENT_CONFIGURED:
            /* Check the configuration */
            configurationValue = ((USB_DEVICE_EVENT_DATA_CONFIGURED *)eventData)->configurationValue;
            if (configurationValue == 1)
            {
                /* The device is in configured state. */

                /* Register the CDC Device application event handler here.
                 * Note how the appData object pointer is passed as the
                 * user data */
                USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0,
                        APP_USBDeviceCDCEventHandler, (uintptr_t)&appData);

                /* mark that set configuration is complete */
                appData.isConfigured = true;

            }
            break;

        case USB_DEVICE_EVENT_SUSPENDED:
            /*  */

            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:
            /* VBUS was detected. Connect the device */
            USB_DEVICE_Attach (appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:
            /* VBUS was removed. Disconnect the device */
            USB_DEVICE_Detach(appData.deviceHandle);
            break;

        /* These events are not used in this project */
        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:
            break;
    }
}


/*******************************************************************************
 * USB Interrupt handler Functions:
 ******************************************************************************/

void __ISR(_USB_VECTOR, ipl6SRS) _IntHandlerUSBInstance0(void)
{
    DRV_USBHS_Tasks_ISR(sysObj.drvUSBObject);
}

void __ISR (_USB_DMA_VECTOR,ipl6SRS) _IntHandlerUSBInstance0_USBDMA ( void )
{
    DRV_USBHS_Tasks_ISR_USBDMA(sysObj.drvUSBObject);
}

