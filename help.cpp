#include "header.h"

void ShowHelp()
{
  Verbose(R"(
MTEE v2.7 Commandline Standard Stream Splitter for Windows.
Copyright (c) 2001-2016 Ritchie Lawrence, http://www.commandline.co.uk.

  MTEE [/O | /U] [/C] [/D] [/T] [/E] [/A] [file...]

  /O    Convert output from UCS-2 to OEM encoding. Default output is same as input.
  /C    Continue if errors occur opening/writing to file (advanced users only).
  /D    Prefix each line of output with local date in YYYY-MM-DD format.
  /T    Prefix each line of output with local time in HH:MM:SS.MSS format.
  /U    Convert output from OEM to UCS-2 encoding. Default output is same as input.
  /E    Exit with exit code of piped process.
  /ET   Calculate and display elapsed time.
  /CPU  Calculate and display average CPU load during execution.
  /A    Append to existing file. If omitted, existing file is overwritten.
  file  File to receive the output. Files are overwritten if /A not specified.
  
  Example usage:

  1) script.cmd | mtee result.txt
  2) ftp -n -s:ftp.scr | mtee local.log | mtee /a \\\\server\\logs$\\remote.log
  3) update.cmd 2>&1 | mtee/d/t/a log.txt

  1) Sends the output of script.cmd to the console and to result.log. If
     result.txt already exists, it will be overwritten.
  2) Sends output of automated ftp session to the console and two log files,
     local.log is overwritten if it already exists, remote.log is appended to.
  3) Redirects stdout and stderr from update.cmd to console and appends to
     log.txt. Each line is prefixed with local date and time.
)");
  ExitProcess(1);
}
