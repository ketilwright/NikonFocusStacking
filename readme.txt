This rev 0.2 of NikType004 Arduino sketch. The only function it currently provides is focus stacking on Nikon cameras.
It was  developed using a Nikon D7000 camera, but may operate with other type0004 Nikons as described in the Nikon
SDKs. Type0004 cameras appear to be the D5100, D90 and D7000.

Dependencies:

1) USB_Host_Shield library at https://github.com/felis/USB_Host_Shield_2.0
2) PTP_2.0 library at https://github.com/ketilwright/PTP_2.0. This is a fork
   from https://github.com/felis/PTP_2.0.

There are some problems to overcome with the Nikon DSLRs which make it a less than ideal camera for tethered focus stacking:

1) Live View must be enabled in order for the PTP_OC_NIKON_MfDrive operation to work.
2) Auto focus must be enabled on the camera in order for the PTP_OC_NIKON_MfDrive to work.
3) PTP_OC_InitiateCapture will not work, since it will attempt to autofocus, defeating the purpose.
4) NK_OC_CaptureInSDRAM is not useful on the arduino to the lack of storage.
4) Many Nikon operations require repeated calls to NK_OC_DeviceReady before proceeding with subsequent ops.

One solution, implemented here, uses the PTP_OC_NIKON_CaptureRecInMedia, specifying 0xffffffff for the
first PTP parameter (indicating no autofocus), and 0 for the 2nd PTP parameter, (indicating to save
to the card).

Focus stacking then becomes a matter of issuing the following operations:

1) Assume we begin with live view off.
2) PTP_OC_NIKON_CaptureRecInMedia
3) Wait for the PTP_EC_CaptureComplete event.
3) Enable live view.
4) Issue NK_OC_DeviceReady repeatedly until the camera returns PTP_RC_OK.
5) Issue PTP_OC_NIKON_MfDrive.
6) Disable live view
7) return to step 2, until all frames are captured.

Note that during the entire operation, the live view is turning on and off. When it is on,
you cannot view through the viewfinder, nor does the liveview image appear on the camera LCD.




