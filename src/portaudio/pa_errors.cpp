/* this file is GO specifc to aide translations - it contains all portaudio error strings */
/*
 * Portable Audio I/O Library Multi-Host API front end
 * Validate function parameters and manage multiple host APIs.
 *
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 1999-2008 Ross Bencina, Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */

result = _("Success");
result = _("PortAudio not initialized");
result = _("Unanticipated host error");
result = _("Invalid number of channels");
result = _("Invalid sample rate");
result = _("Invalid device");
result = _("Invalid flag");
result = _("Sample format not supported");
result = _("Illegal combination of I/O devices");
result = _("Insufficient memory");
result = _("Buffer too big");
result = _("Buffer too small");
result = _("No callback routine specified");
result = _("Invalid stream pointer");
result = _("Wait timed out");
result = _("Internal PortAudio error");
result = _("Device unavailable");
result = _("Incompatible host API specific stream info");
result = _("Stream is stopped");
result = _("Stream is not stopped");
result = _("Input overflowed");
result = _("Output underflowed");
result = _("Host API not found");
result = _("Invalid host API");
result = _("Can't read from a callback stream");
result = _("Can't write to a callback stream");
result = _("Can't read from an output only stream");
result = _("Can't write to an input only stream");
result = _("Incompatible stream host API");
result = _("Bad buffer pointer");
result = _("Invalid error code (value greater than zero)"); 
result = _("Invalid error code"); 

name = _("ALSA");
name = _("ASIO");
name = _("Core Audio");
name = _("Windows DirectSound");
name = _("JACK Audio Connection Kit");
name = _("OSS");
name = _("Windows WASAPI");
name = _("Windows WDM-KS");
name = _("MME");
