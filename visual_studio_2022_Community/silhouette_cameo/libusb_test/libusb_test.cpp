#include <iostream>
#include <iomanip>
#include <string.h>
#include <libusb.h>

// USB vendor and product id for the Silhouette Cameo 4
// 
// This code is stolen from: https://github.com/Timmmm/robocut/blob/master/Plotter.cpp
// 
// https://the-sz.com/products/usbid/
// 0x0b4d, 0x112b # Silhouette Cameo 2
// 0x0B4D, 0x1137 speed: 12M # Vendor: Graphtec

// Setting up the driver for libusb on windows
// 
// Use Zadig.exe on windows to activate the driver "" for the device
// Options > List All Devices 
// Select "USB Printing Support" with USB ID: 0x0B4D, 0x1137 from the list box
// Activate the driver "WinUSB (v6.1.7600.16385)" by selecting it and clicking "Replace Driver"

// ERROR: libusb writing to endpoint produces "entity not found" error code
// SOLUTION: before writing data to an interface, the application has to claim and unclaim the
// interface by calling libusb_claim_interface() and libusb_release_interface() afterwards.

// ERROR: Querying the status from the Cameo results in the status code 0x32 which is not one of the valid codes
// SOLUATION: ??? Maybe make sure that all hardware cutter knifes are installed correctly? 
// Make sure the device works using the official tools.

// https://www.ohthehugemanatee.net/2011/07/gpgl-reference-courtesy-of-graphtec/

// robocut / Plotter.h:53 ff
#define VENDOR_ID_GRAPHTEC 0x0b4d

#define PRODUCT_ID_CC200_20 0x110a
#define PRODUCT_ID_CC300_20 0x111a

#define PRODUCT_ID_SILHOUETTE_SD_1 0x111c
#define PRODUCT_ID_SILHOUETTE_SD_2 0x111d

#define PRODUCT_ID_SILHOUETTE_CAMEO 0x1121
#define PRODUCT_ID_SILHOUETTE_CAMEO2 0x112b
#define PRODUCT_ID_SILHOUETTE_CAMEO3 0x112f
#define PRODUCT_ID_SILHOUETTE_CAMEO4 0x1137
#define PRODUCT_ID_SILHOUETTE_CAMEO4PLUS 0x1138
#define PRODUCT_ID_SILHOUETTE_CAMEO4PRO 0x1139
#define PRODUCT_ID_SILHOUETTE_CAMEO5 0x1140
#define PRODUCT_ID_SILHOUETTE_CAMEO5PLUS 0x1141

#define PRODUCT_ID_SILHOUETTE_PORTRAIT 0x1123
#define PRODUCT_ID_SILHOUETTE_PORTRAIT2 0x1132
#define PRODUCT_ID_SILHOUETTE_PORTRAIT3 0x113a

// output verbose information about the found USB devices
int verbose = 1;

static void print_endpoint_comp(const struct libusb_ss_endpoint_companion_descriptor* ep_comp);
static void print_endpoint(const struct libusb_endpoint_descriptor* endpoint);
static void print_altsetting(const struct libusb_interface_descriptor* interface);
static void print_2_0_ext_cap(struct libusb_usb_2_0_extension_descriptor* usb_2_0_ext_cap);
static void print_ss_usb_cap(struct libusb_ss_usb_device_capability_descriptor* ss_usb_cap);
static void print_bos(libusb_device_handle* handle);
static void print_interface(const struct libusb_interface* interface);
static void print_configuration(struct libusb_config_descriptor* config);
static void print_device(libusb_device* dev, libusb_device_handle* handle);
static int test_wrapped_device(const char* device_name);

static void cleanup(libusb_device** devs, libusb_device_handle** device);
static int write(libusb_device_handle* handle, unsigned char endpoint, unsigned char* data,
    unsigned int data_length, int* transferred, int timeout);

std::string UsbError(int e)
{
    switch (e)
    {
    case LIBUSB_SUCCESS:
        return "Success (internal error)";
    case LIBUSB_ERROR_IO:
        return "I/O error";
    case LIBUSB_ERROR_INVALID_PARAM:
        return "Invalid parameter";
    case LIBUSB_ERROR_ACCESS:
        return "Access denied";
    case LIBUSB_ERROR_NO_DEVICE:
        return "No device";
    case LIBUSB_ERROR_NOT_FOUND:
        return "Entity not found";
    case LIBUSB_ERROR_BUSY:
        return "Resource busy";
    case LIBUSB_ERROR_TIMEOUT:
        return "Timeout";
    case LIBUSB_ERROR_OVERFLOW:
        return "Overflow";
    case LIBUSB_ERROR_PIPE:
        return "Pipe error";
    case LIBUSB_ERROR_INTERRUPTED:
        return "Syscall interrupted";
    case LIBUSB_ERROR_NO_MEM:
        return "Insufficient memory";
    case LIBUSB_ERROR_NOT_SUPPORTED:
        return "Operation not supported";
    case LIBUSB_ERROR_OTHER:
        return "Other error";
    }
    return "Unknown error";
}

int main()
{
    std::cout << "Hello LibUSB!" << std::endl;

    int result = libusb_init(NULL);
    if (result < 0)
    {
        std::cout << "libusb_init() failed! Error code: " << result << " Aborting!" << std::endl;

        return result;
    }

    std::cout << "libusb_init() succeeded!" << std::endl;

    libusb_device** devs = NULL;
    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
    {
        std::cout << "libusb_get_device_list() did not find any devices" << std::endl;
        libusb_exit(NULL);
        return 1;
    }

    libusb_device* target_device = NULL;
    for (int i = 0; devs[i]; i++)
    {
        libusb_device* device = devs[i];

        // DEBUG output all devices that windows/libusb currently lists
        //print_device(device, NULL);

        struct libusb_device_descriptor desc;
        int ret = libusb_get_device_descriptor(device, &desc);
        if (ret < 0)
        {
            fprintf(stderr, "failed to get device descriptor");
            continue;
        }

        if ((desc.idVendor == VENDOR_ID_GRAPHTEC) && (desc.idProduct == PRODUCT_ID_SILHOUETTE_CAMEO4))
        {
            std::cout << "Silhouette Cameo 4 found!" << std::endl;
            target_device = device;
        }
    }

    if (target_device == NULL)
    {
        std::cout << "Could not find Silhouette Cameo 4!" << std::endl;

        libusb_free_device_list(devs, 1);
        libusb_exit(NULL);

        return 0;
    }

    print_device(target_device, NULL);

    // if you open a device, you will get a handle to the device 
    // which you have to close when you are done using the device

    libusb_device_handle* target_device_handle = NULL;
    libusb_open(target_device, &target_device_handle);





    // use device here ...



    int ret = 0;
    int transferred;
    int timeout = 0; // timeout in ms, 0 means no timeout

    const int interface_index = 0;



    //
    // Claim interface (returns 0 on success)
    // 
    // Call libusb_release_interface() to release the interface after usage.
    //

    std::cout << "Claiming main control interface." << std::endl;
    ret = libusb_claim_interface(target_device_handle, interface_index);
    if (ret < 0)
    {
        cleanup(devs, &target_device_handle);

        std::cerr << "Error claiming the interface: " << UsbError(ret) << std::endl;
        return -1;
    }

    //
    // Go through the initialization sequence.
    // 
    // The sequence is documented here: https://github.com/fablabnbg/inkscape-silhouette/blob/main/Commands.md
    // Scroll to the end of the page to find the initializiation sequence
    // 
    // Sample code that implements the sequence is available here: https://github.com/Timmmm/robocut/blob/master/Plotter.cpp
    // Sample code in python that implements the sequence is available in the start() function here: https://github.com/pmonta/gerber2graphtec/blob/master/graphtec.py
    // 
    // A part of the initialization sequence is the handling registration marks.
    // What are registration marks? 
    // Registration marks are extra markings that are added to a design or print and cut. 
    // They need to be intentionally added. The two different types of registration marks are: 
    // print and cut registration marks and vinyl layering registration marks.
    // Registration marks are symbols added to printing plates or screens to ensure proper alignment 
    // of colors during the printing process. These marks are especially crucial in multi-color printing 
    // to make sure that each color layer aligns accurately with the others.
    // You'll need to add these types of registration marks so that the cutter knows exactly where to cut 
    // after you've completed the print on your printer. Most often print and cut registration marks are 
    // used for making stickers, full color printed embellishments, and printable heat transfer and printable vinyl decals.
    // 
    // The initialization sequence is executed until the command that switches the system to data mode
    // page << "&100,100,100,\\0,0,Z" << ItoS(width) << "," << ItoS(height) << ",L0";
    // 
    // Once in data mode, the sample code loops through all cut objects.
    // Each cut object is executed on the cameo.
    // Once all cuts have been executed, data mode is exited
    // "&1,1,1,TB50,0\x03"	# ??			# Back from Data mode.
    // 
    // 
    // 
    // The basic idea to perform the cut job is to send path commands to the cutter.
    // A path command is: 'D%.3f,%.3f\x03' % (x,y)
    // See https://github.com/pmonta/gerber2graphtec/blob/master/graphtec.py and the def draw(self, x, y): function.
    // 
    // def draw(self, x, y):
    //    x,y = self.transform(x,y)
    //    self.emit('D%.3f,%.3f\x03' % (x, y))
    // 
    // The D command in the silouhette cameo 4 language is described as:
    // Draw                   (G)[t]  Cuts from the current position to each of the given points in turn
    // Example: Dxa,ya,xb,yb,...,xn,yn goes over the coordinate pairs (xa, ya), (xb, yb) ... (xn, yn) and
    // cuts that path.
    // 
    // This means a path is a list of points and the cutter will go ahead an cut from point to point
    // in straight lines.
    // 
    // A possible model for a path is a polygon. A polygon is a list of points that are then connected by lines to
    // form a shape. The robocut software uses the QPolygonF class from teh Qt library to manage paths.
    // A simpler way is just to store a list of (x, y) coordinate pairs for the path.
    // 
    // The software has to apply transforms (which???) to the path or individually to every (x, y) coordinate
    // pair and then just send D commands to the cutter in order to execute the path.
    // 
    // To cut out a square, the path would describe all four corners and then eventually run a last line
    // from the last corner back the first corner to cut out the last edge of the square. 
    // 
    // 
    // 
    // Converting from gerber to polygons:
    // The question is where to receive the paths / ploygons from so they can be sent to the plotter.
    // A library needs to exist that can convert a gerber file to paths / polygons.
    // This library needs to extract closed paths in the gerber code and just export them as a list
    // of (x, y) coordinates. Then formulate path commands and send them to the cameo cutter using
    // libusb.
    // 
    // A second approach is to use existing applications (gerbv, pstoedit, ...) instead of 
    // code libraries and write an application that remote controls these binary applications.
    // First use the gerbv tool to export the .gbr gerber file to pdf.
    // Then use pstoedit to convert the pdf file to a vector format .pic file.
    // Then use a .pic library to read back in the .pic file and convert it to strokes
    // and send those to a library that executes the strokes as paths on the silhouette cameo
    // using libusb.
    // This is the approach used by https://github.com/pmonta/gerber2graphtec/blob/master/gerber2graphtec
    // 
    // 
    //
    // Once data mode is exited, the page is pushed out of the cutter.
    // "FO0\x03"		# feed the page out
    // 
    // Then the cutter is told to stop operation
    // "H,"         		# halt ?
    // 



    // Initialise plotter.
    //e = UsbSend(handle, "\x1b\x04");
    //if (!e) goto error;

    // why use this endpoint ???
    unsigned char tx_endpoint = '\x01';


    const int data_length = 2;
    unsigned char data[data_length];
    data[0] = 0x1B;
    data[1] = 0x04;

    ret = write(target_device_handle,
        tx_endpoint,
        data,
        data_length,
        &transferred,
        timeout);
    if ((ret != 0) || ((unsigned int)transferred != data_length))
    {
        libusb_release_interface(target_device_handle, interface_index);
        cleanup(devs, &target_device_handle);
        return -1;
    }





    //
    // Status request.
    //

    //e = UsbSend(handle, "\x1b\x05");
    //if (!e) goto error;

    const int data_length_2 = 2;
    unsigned char data_2[data_length_2];
    data_2[0] = 0x1B;
    data_2[1] = 0x05;

    ret = write(target_device_handle,
        tx_endpoint,
        data_2,
        data_length_2,
        &transferred,
        timeout);
    if ((ret != 0) || ((unsigned int)transferred != data_length))
    {
        libusb_release_interface(target_device_handle, interface_index);
        cleanup(devs, &target_device_handle);
        return -1;
    }

    //e = UsbReceive(handle, resp, 5000);
    //if (!e) goto error;

    //if (resp != "0\x03") // 0 = Ready. 1 = Moving. 2 = Nothing loaded. "  " = ??
    //{
    //    if (resp == "1\x03")
    //        e = Error("Moving, please try again.");
    //    else if (resp == "2\x03")
    //        e = Error("Empty tray, please load media.");	// Silhouette Cameo
    //    else
    //        e = Error("Invalid response from plotter: " + resp);
    //    goto error;
    //}

    //s.clear();

    // A buffer that is one packet long.
    const int PacketSize = 64;
    unsigned char buffer[PacketSize];
    //int transferred = 0;
    unsigned char rx_endpoint = '\x82';
    ret = libusb_bulk_transfer(target_device_handle, rx_endpoint, buffer, PacketSize, &transferred, 5000);
    // But it could be a timeout.
    if (ret != 0)
    {
        std::cerr << "Error reading from device: " << UsbError(ret) << std::endl;
    }

    std::cout << "<< " << std::endl;
    for (int i = 0; i < transferred; i++)
    {
        std::cout << "data: " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i]) << std::endl;
    }

    if (buffer[0] == 0x00)
    {
        std::cout << "[SILHOUETTE_CAMEO_4] Status: Ready" << std::endl;
    }
    else if (buffer[0] == 0x01)
    {
        std::cout << "[SILHOUETTE_CAMEO_4] Status: Moving" << std::endl;
    }
    else if (buffer[0] == 0x02)
    {
        std::cout << "[SILHOUETTE_CAMEO_4] Status: Nothing Loaded" << std::endl;
    }
    else if (buffer[0] == 0x32)
    {
        std::cout << "[SILHOUETTE_CAMEO_4] Status: Setup or USB communication problem!" << std::endl;
    }
    else
    {
        std::cout << "[SILHOUETTE_CAMEO_4] Status: UNKNOWN STATUS" << std::endl;
    }





    //
    // Query the printer's firmware version
    // 

    //e = UsbSend(handle, "FG\x03");

    const int data_length_3 = 3;
    unsigned char data_3[data_length_3];
    data_3[0] = 0x46; // F
    data_3[1] = 0x47; // G
    data_3[2] = 0x03; // 0x03

    ret = write(target_device_handle,
        tx_endpoint,
        data_3,
        data_length_3,
        &transferred,
        timeout);
    if ((ret != 0) || ((unsigned int)transferred != data_length_3))
    {
        libusb_release_interface(target_device_handle, interface_index);
        cleanup(devs, &target_device_handle);
        return -1;
    }

    //Sleep(4000);

    // Receive the firmware version.

    // A buffer that is one packet long.
    ret = libusb_bulk_transfer(target_device_handle, rx_endpoint, buffer, PacketSize, &transferred, 0);
    // But it could be a timeout.
    if (ret != 0)
    {
        std::cerr << "Error reading from device: " << UsbError(ret) << std::endl;
    }

    // output buffer as hex
    std::cout << "<<";
    for (int i = 0; i < transferred; i++)
    {
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i]);
    }
    std::cout << std::endl;

    // output buffer as ASCII
    std::string result_as_string = std::string(buffer, buffer + transferred);
    std::cout << "<< " << result_as_string << std::endl;



    //
    // Release / Unclaim interface
    //

    libusb_release_interface(target_device_handle, interface_index);



    // clean up
    cleanup(devs, &target_device_handle);

    return 0;
}

static int write(libusb_device_handle* handle, unsigned char endpoint, unsigned char* data,
    unsigned int data_length, int* transferred, int timeout)
{
    int ret = libusb_bulk_transfer(handle,
        endpoint,
        //reinterpret_cast<unsigned char*>(const_cast<char*>(data.c_str())),
        data,
        //data.length(),
        data_length,
        transferred,
        timeout);

    if (ret != 0)
    {
        //std::cerr << "Error writing to device: " << UsbError(ret) << std::endl;
        //return Error("Error writing to device: " + UsbError(ret));

        std::cerr << "Error writing to device: " << UsbError(ret) << std::endl;

        return -1;
    }

    //if ((unsigned int)transferred != data.length())
    if ((unsigned int)*transferred != data_length)
    {
        std::cerr << "Warning, some data not transferred correctly." << std::endl;
        //return Error("Some data not transfered. Attempted: " + ItoS(data.length()) + " Transferred: " + ItoS(transferred));

        return -1;
    }

    return 0;
}

static void cleanup(libusb_device** devs, libusb_device_handle** device)
{
    if (device != NULL)
    {
        libusb_close(*device);
        *device = NULL;
    }

    if (devs != NULL)
    {
        libusb_free_device_list(devs, 1);
    }

    libusb_exit(NULL);
}

static void print_endpoint_comp(const struct libusb_ss_endpoint_companion_descriptor* ep_comp)
{
    printf("      USB 3.0 Endpoint Companion:\n");
    printf("        bMaxBurst:           %u\n", ep_comp->bMaxBurst);
    printf("        bmAttributes:        %02xh\n", ep_comp->bmAttributes);
    printf("        wBytesPerInterval:   %u\n", ep_comp->wBytesPerInterval);
}

static void print_endpoint(const struct libusb_endpoint_descriptor* endpoint)
{
    int i, ret;

    printf("      Endpoint:\n");
    printf("        bEndpointAddress:    %02xh\n", endpoint->bEndpointAddress);
    printf("        bmAttributes:        %02xh\n", endpoint->bmAttributes);
    printf("        wMaxPacketSize:      %u\n", endpoint->wMaxPacketSize);
    printf("        bInterval:           %u\n", endpoint->bInterval);
    printf("        bRefresh:            %u\n", endpoint->bRefresh);
    printf("        bSynchAddress:       %u\n", endpoint->bSynchAddress);

    for (i = 0; i < endpoint->extra_length;)
    {
        if (LIBUSB_DT_SS_ENDPOINT_COMPANION == endpoint->extra[i + 1])
        {
            struct libusb_ss_endpoint_companion_descriptor* ep_comp;

            ret = libusb_get_ss_endpoint_companion_descriptor(NULL, endpoint, &ep_comp);
            if (LIBUSB_SUCCESS != ret)
                continue;

            print_endpoint_comp(ep_comp);

            libusb_free_ss_endpoint_companion_descriptor(ep_comp);
        }

        i += endpoint->extra[i];
    }
}

static void print_altsetting(const struct libusb_interface_descriptor* interface)
{
    uint8_t i;

    printf("    Interface:\n");
    printf("      bInterfaceNumber:      %u\n", interface->bInterfaceNumber);
    printf("      bAlternateSetting:     %u\n", interface->bAlternateSetting);
    printf("      bNumEndpoints:         %u\n", interface->bNumEndpoints);
    printf("      bInterfaceClass:       %u\n", interface->bInterfaceClass);
    printf("      bInterfaceSubClass:    %u\n", interface->bInterfaceSubClass);
    printf("      bInterfaceProtocol:    %u\n", interface->bInterfaceProtocol);
    printf("      iInterface:            %u\n", interface->iInterface);

    for (i = 0; i < interface->bNumEndpoints; i++)
        print_endpoint(&interface->endpoint[i]);
}

static void print_2_0_ext_cap(struct libusb_usb_2_0_extension_descriptor* usb_2_0_ext_cap)
{
    printf("    USB 2.0 Extension Capabilities:\n");
    printf("      bDevCapabilityType:    %u\n", usb_2_0_ext_cap->bDevCapabilityType);
    printf("      bmAttributes:          %08xh\n", usb_2_0_ext_cap->bmAttributes);
}

static void print_ss_usb_cap(struct libusb_ss_usb_device_capability_descriptor* ss_usb_cap)
{
    printf("    USB 3.0 Capabilities:\n");
    printf("      bDevCapabilityType:    %u\n", ss_usb_cap->bDevCapabilityType);
    printf("      bmAttributes:          %02xh\n", ss_usb_cap->bmAttributes);
    printf("      wSpeedSupported:       %u\n", ss_usb_cap->wSpeedSupported);
    printf("      bFunctionalitySupport: %u\n", ss_usb_cap->bFunctionalitySupport);
    printf("      bU1devExitLat:         %u\n", ss_usb_cap->bU1DevExitLat);
    printf("      bU2devExitLat:         %u\n", ss_usb_cap->bU2DevExitLat);
}

static void print_bos(libusb_device_handle* handle)
{
    struct libusb_bos_descriptor* bos;
    uint8_t i;
    int ret;

    ret = libusb_get_bos_descriptor(handle, &bos);
    if (ret < 0)
        return;

    printf("  Binary Object Store (BOS):\n");
    printf("    wTotalLength:            %u\n", bos->wTotalLength);
    printf("    bNumDeviceCaps:          %u\n", bos->bNumDeviceCaps);

    for (i = 0; i < bos->bNumDeviceCaps; i++)
    {
        struct libusb_bos_dev_capability_descriptor* dev_cap = bos->dev_capability[i];

        if (dev_cap->bDevCapabilityType == LIBUSB_BT_USB_2_0_EXTENSION)
        {
            struct libusb_usb_2_0_extension_descriptor* usb_2_0_extension;

            ret = libusb_get_usb_2_0_extension_descriptor(NULL, dev_cap, &usb_2_0_extension);
            if (ret < 0)
                return;

            print_2_0_ext_cap(usb_2_0_extension);
            libusb_free_usb_2_0_extension_descriptor(usb_2_0_extension);
        }
        else if (dev_cap->bDevCapabilityType == LIBUSB_BT_SS_USB_DEVICE_CAPABILITY)
        {
            struct libusb_ss_usb_device_capability_descriptor* ss_dev_cap;

            ret = libusb_get_ss_usb_device_capability_descriptor(NULL, dev_cap, &ss_dev_cap);
            if (ret < 0)
                return;

            print_ss_usb_cap(ss_dev_cap);
            libusb_free_ss_usb_device_capability_descriptor(ss_dev_cap);
        }
    }

    libusb_free_bos_descriptor(bos);
}

static void print_interface(const struct libusb_interface* interface)
{
    int i;

    for (i = 0; i < interface->num_altsetting; i++)
        print_altsetting(&interface->altsetting[i]);
}

static void print_configuration(struct libusb_config_descriptor* config)
{
    uint8_t i;

    printf("  Configuration:\n");
    printf("    wTotalLength:            %u\n", config->wTotalLength);
    printf("    bNumInterfaces:          %u\n", config->bNumInterfaces);
    printf("    bConfigurationValue:     %u\n", config->bConfigurationValue);
    printf("    iConfiguration:          %u\n", config->iConfiguration);
    printf("    bmAttributes:            %02xh\n", config->bmAttributes);
    printf("    MaxPower:                %u\n", config->MaxPower);

    for (i = 0; i < config->bNumInterfaces; i++)
        print_interface(&config->interface[i]);
}

static void print_device(libusb_device* dev, libusb_device_handle* handle)
{
    struct libusb_device_descriptor desc;
    unsigned char string[256];
    const char* speed;
    int ret;
    uint8_t i;

    switch (libusb_get_device_speed(dev))
    {
    case LIBUSB_SPEED_LOW:		speed = "1.5M"; break;
    case LIBUSB_SPEED_FULL:		speed = "12M"; break;
    case LIBUSB_SPEED_HIGH:		speed = "480M"; break;
    case LIBUSB_SPEED_SUPER:	speed = "5G"; break;
    case LIBUSB_SPEED_SUPER_PLUS:	speed = "10G"; break;
    default:			speed = "Unknown";
    }

    ret = libusb_get_device_descriptor(dev, &desc);
    if (ret < 0)
    {
        fprintf(stderr, "failed to get device descriptor");
        return;
    }

    printf("Dev (bus %u, device %u): %04X - %04X speed: %s\n",
        libusb_get_bus_number(dev), libusb_get_device_address(dev),
        desc.idVendor, desc.idProduct, speed);

    if (!handle)
        libusb_open(dev, &handle);

    if (handle)
    {
        if (desc.iManufacturer)
        {
            ret = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, string, sizeof(string));
            if (ret > 0)
                printf("  Manufacturer:              %s\n", (char*)string);
        }

        if (desc.iProduct)
        {
            ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string, sizeof(string));
            if (ret > 0)
                printf("  Product:                   %s\n", (char*)string);
        }

        if (desc.iSerialNumber && verbose)
        {
            ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, string, sizeof(string));
            if (ret > 0)
                printf("  Serial Number:             %s\n", (char*)string);
        }
    }

    if (verbose)
    {
        for (i = 0; i < desc.bNumConfigurations; i++)
        {
            struct libusb_config_descriptor* config;

            ret = libusb_get_config_descriptor(dev, i, &config);
            if (LIBUSB_SUCCESS != ret)
            {
                printf("  Couldn't retrieve descriptors\n");
                continue;
            }

            print_configuration(config);

            libusb_free_config_descriptor(config);
        }

        if (handle && desc.bcdUSB >= 0x0201)
            print_bos(handle);
    }

    if (handle)
        libusb_close(handle);
}

static int test_wrapped_device(const char* device_name)
{
    (void)device_name;
    printf("Testing wrapped devices is not supported on your platform\n");
    return 1;
}