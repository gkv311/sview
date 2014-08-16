/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMonitorsDump program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMonitorsDump program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <StThreads/StProcess.h>
#include <StCore/StSearchMonitors.h>
#include <StStrings/stConsole.h>
#include <StFile/StRawFile.h>
#include <StVersion.h>

#include <fstream>

namespace {

static StString formatHex(const unsigned char* theData, size_t theSize) {
    StString anOut;
    stUtf8_t aByte[4];
    for(size_t aByteId = 0; aByteId < theSize; ++aByteId) {
        unsigned char aChar = theData[aByteId];
        char anEsc = ' ';
        if(       (aByteId + 1) % 16 == 0 && aByteId != 0) {
            anEsc = '\n';
        } else if((aByteId + 1) % 8  == 0) {
            anEsc = '|';
        }
        // TODO (Kirill Gavrilov#3#) got the strange crash on Win64 with setted buffer size to 3...
        stsprintf(aByte, 4, "%02X%c", (unsigned int )aChar, anEsc);
        anOut += StString(aByte);
    }
    return anOut;
}

static StString dump(const StSearchMonitors& theMonitors) {
    StString aStrDump;
    for(size_t aMonIter = 0; aMonIter < theMonitors.size(); ++aMonIter) {
        aStrDump += theMonitors[aMonIter].toString();
        aStrDump += '\n';
    }
    return aStrDump;
}

static StString dumpEdid(const StEDIDParser& theEdid) {
    if(!theEdid.isValid()) {
        return StString("== INVALID EDID ==\n");
    }

    StString aDumpStr;
    aDumpStr += StString("== Monitor ") + theEdid.getPnPId() + " ============================\n";
    aDumpStr += StString("== Name:       ") + theEdid.getName() + "\n";
    aDumpStr += StString("== Year/Week:  ") + theEdid.getYear() + "/" + theEdid.getWeek() + "\n";
    aDumpStr += StString("== Gamma:      ") + theEdid.getGamma() + "\n";
    aDumpStr += StString("== Stereo:     ") + theEdid.getStereoString() + "\n";
    aDumpStr += StString("== Dimensions: ") + theEdid.getWidthMM() + " X " + theEdid.getHeightMM() + " mm\n";
    aDumpStr += StString("================= EDID data ===================\n");
    aDumpStr += formatHex(theEdid.getData(), theEdid.getSize());
    aDumpStr += StString("===============================================\n");
    return aDumpStr;
}

static void dumpEdidToFile(const StEDIDParser& theEdid,
                           const StString&     theFileName) {
    StRawFile aRawFile(theFileName);
    if(!aRawFile.openFile(StRawFile::WRITE)) {
        st::cout << stostream_text("Can not open the file '") << theFileName << stostream_text("' for writing!\n");
        return;
    }
    aRawFile.initBuffer(theEdid.getSize());
    stMemCpy(aRawFile.changeBuffer(), theEdid.getData(), theEdid.getSize());
    aRawFile.writeFile(theEdid.getSize());
    aRawFile.closeFile();
}

static void genInf(const StEDIDParser& theEdid,
                   const StString&     theFileName) {
    st::ofstream aFileOut;
    aFileOut.open(theFileName.toCString());
    if(aFileOut.fail()) {
        st::cout << st::COLOR_FOR_RED << stostream_text("Couldn't open file \"") << theFileName << stostream_text("\"!\n") << st::COLOR_FOR_WHITE;
        return;
    }

    aFileOut << "[Version]\n";
    aFileOut << "Signature=\"$WINDOWS NT$\"\n";
    aFileOut << "Class=Monitor\n";
    aFileOut << "ClassGUID={4d36e96e-e325-11ce-bfc1-08002be10318}\n";
    aFileOut << "Provider=%MFG%\n";
    aFileOut << "DriverVer=20.03.2010, 1.0.0.0\n";
    aFileOut << ";CatalogFile=YourSignedCatalogFile.cat\n\n";

    aFileOut << "[DestinationDirs]\n";
    aFileOut << "DefaultDestDir=23\n\n";

    aFileOut << "[SourceDisksNames]\n";
    aFileOut << "1=%DISC%\n\n";

    aFileOut << "[SourceDisksFiles]\n";
    aFileOut << ";YourColorProfileFile.icm\n\n";

    aFileOut << "[Manufacturer]\n";
    aFileOut << "%VENDOR%=EDID_OVERRIDE,NTx86,NTamd64\n\n";

    aFileOut << "[EDID_OVERRIDE.NTx86]\n";
    aFileOut << "%PRODUCTID%=OVERRIDDEN-EDID.Install, MONITOR\\" << theEdid.getPnPId() << "\n\n";

    aFileOut << "[EDID_OVERRIDE.NTamd64]\n";
    aFileOut << "%PRODUCTID%=OVERRIDDEN-EDID.Install.NTamd64, MONITOR\\" << theEdid.getPnPId() << "\n\n";

    aFileOut << "[OVERRIDDEN-EDID.Install.NTx86]\n";
    aFileOut << "DelReg=DEL_CURRENT_REG\n";
    aFileOut << "AddReg=OVERRIDDEN-EDID.AddReg, MODE1, DPMS\n";
    aFileOut << "CopyFiles=OVERRIDDEN-EDID.CopyFiles\n\n";

    aFileOut << "[OVERRIDDEN-EDID.Install.NTamd64]\n";
    aFileOut << "DelReg=DEL_CURRENT_REG\n";
    aFileOut << "AddReg=OVERRIDDEN-EDID.AddReg, MODE1, DPMS\n";
    aFileOut << "CopyFiles=OVERRIDDEN-EDID.CopyFiles\n\n";

    aFileOut << "[OVERRIDDEN-EDID.Install.NTx86.HW]\n";
    aFileOut << "AddReg=OVERRIDDEN-EDID_AddReg\n\n";

    aFileOut << "[OVERRIDDEN-EDID.Install.NTamd64.HW]\n";
    aFileOut << "AddReg=OVERRIDDEN-EDID_AddReg\n\n";

    aFileOut << "[OVERRIDDEN-EDID_AddReg]\n";
    aFileOut << ";Base EDID\n";
    aFileOut << "HKR,EDID_OVERRIDE,\"0\",0x01";
    stUtf8_t aByte[4];
    for(size_t aByteId = 0; aByteId < theEdid.getSize(); ++aByteId) {
        unsigned char aChar = theEdid.getData()[aByteId];
        stsprintf(aByte, 4, "%02X", (unsigned int )aChar);
        aFileOut << ",0x" << aByte;
    }
    aFileOut << "\n\n";

    aFileOut << "[DEL_CURRENT_REG]\n";
    aFileOut << "HKR,MODES\n";
    aFileOut << "HKR,EDID_OVERRIDE\n";
    aFileOut << "HKR,,MaxResolution\n";
    aFileOut << "HKR,,DPMS\n";
    aFileOut << "HKR,,ICMProfile\n\n";

    aFileOut << "[DPMS]\n";
    aFileOut << "HKR,,DPMS,,0\n";

    aFileOut << "[MODE1]\n";
    aFileOut << "HKR,,MaxResolution,,\"1280,720\"\n\n";

    aFileOut << "[OVERRIDDEN-EDID.AddReg]\n";
    aFileOut << "HKR,\"MODES\\1280,720\",Mode1,,\"30.0-100.0,50.0-120.0,+,+\"\n\n";

    aFileOut << "[OVERRIDDEN-EDID.CopyFiles]\n";
    aFileOut << ";YourColorProfileFile.icm\n\n";

    aFileOut << "[Strings]\n";
    aFileOut << "MFG=\"EnTech Taiwan\"\n";
    aFileOut << "DISC=\"Monitor EDID Override Installation Disk\"\n";
    aFileOut << "PRODUCTID=\"" << theEdid.getName() << " as " << theEdid.getPnPId() << " (EDID Override)\"\n";
    aFileOut << "VENDOR=\"Acer\"\n";
    aFileOut.close();
}

};

int main(int , char** ) { // force console output
#ifdef _WIN32
    setlocale(LC_ALL, ".OCP"); // we set default locale for console output (useful only for debug)
#endif

    const StString ARGUMENT_ANY        = "--";
    const StString ARGUMENT_SET_PNPID  = "setPnPid";
    const StString ARGUMENT_SET_PNPID2 = "setId";
    const StString ARGUMENT_IN_EDID    = "in";
    const StString ARGUMENT_OUT_EDID   = "out";
    const StString ARGUMENT_OUT_INF    = "genDriver";
    const StString ARGUMENT_OUT_INF2   = "genInf";
    const StString ARGUMENT_HELP       = "help";

    StString aPnPIdReplace, anOutEdidFilename, anOutInfFilename;
    StEDIDParser anInputEdid;

    StArrayList<StString> anArgs = StProcess::getArguments();
    for(size_t aParamIter = 1; aParamIter < anArgs.size(); ++aParamIter) {
        StString aParam = anArgs[aParamIter];
        if(!aParam.isStartsWith(ARGUMENT_ANY)) {
            continue;
        }
        StArgument anArg; anArg.parseString(aParam.subString(2, aParam.getLength())); // cut suffix --
        if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_SET_PNPID)
        || anArg.getKey().isEqualsIgnoreCase(ARGUMENT_SET_PNPID2)) {
            aPnPIdReplace = anArg.getValue();
            if(aPnPIdReplace.getLength() != 7) {
                st::cout << stostream_text("Invalid PnPID ')") << aPnPIdReplace << stostream_text("'\n");
                return -1;
            }
        } else if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_IN_EDID)) {
            if(anInputEdid.isValid()) {
                st::cout << stostream_text("Invalid number of arguments!\n");
                return -1;
            }
            StRawFile aRawFile(anArg.getValue());
            if(!aRawFile.readFile()) {
                st::cout << stostream_text("Can't read the file ')") << anArg.getValue() << stostream_text("'\n");
                return -1;
            } else if(aRawFile.getSize() < 128) {
                st::cout << stostream_text("To small EDID data read from file ')") << anArg.getValue() << stostream_text("'\n");
                return -1;
            }
            anInputEdid.init(aRawFile.getBuffer(), (unsigned int )aRawFile.getSize());
        } else if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_OUT_EDID)) {
            if(!anOutEdidFilename.isEmpty()) {
                st::cout << stostream_text("Invalid number of arguments!\n");
                return -1;
            }
            anOutEdidFilename = anArg.getValue();
        } else if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_OUT_INF)
               || anArg.getKey().isEqualsIgnoreCase(ARGUMENT_OUT_INF2)) {
            if(!anOutInfFilename.isEmpty()) {
                st::cout << stostream_text("Invalid number of arguments!\n");
                return -1;
            }
            anOutInfFilename = anArg.getValue();
        } else if(anArg.getKey().isEqualsIgnoreCase(ARGUMENT_HELP)) {
            st::cout << stostream_text("Usage:\n")
                     << stostream_text("  --help          Show this help\n")
                     << stostream_text("  --setId=AAA0000 New PnPID to override with\n")
                     << stostream_text("  --in=file       Filename for binary EDID dump to read\n")
                     << stostream_text("  --out=file      Filename for binary EDID dump to write\n")
                     << stostream_text("  --genInf=file   Filename for generation of driver INF file\n");
            return 0;
        }
    }

    if(anInputEdid.isValid()) {
        StString aPnPId = anInputEdid.getPnPId();
        st::cout << stostream_text("== EDID data for Monitor with PnPId='") << aPnPId << stostream_text("'==\n");
        st::cout << formatHex(anInputEdid.getData(), anInputEdid.getSize());
        st::cout << stostream_text("===============================================\n");

        if(!aPnPIdReplace.isEmpty() && !aPnPId.isEquals(aPnPIdReplace)) {
            anInputEdid.setPnPId(aPnPIdReplace);
            st::cout << stostream_text("== EDID data for Monitor with PnPId='") << aPnPId << stostream_text("'->'") << anInputEdid.getPnPId() << stostream_text("'==\n");
            st::cout << formatHex(anInputEdid.getData(), anInputEdid.getSize());
            st::cout << stostream_text("===============================================\n");
        }

        if(!anOutEdidFilename.isEmpty()) {
            if(anOutEdidFilename.isEndsWithIgnoreCase(stCString(".bin"))) {
                anOutEdidFilename = anOutEdidFilename.subString(0, anOutEdidFilename.getLength() - 4);
            }
            dumpEdidToFile(anInputEdid, anOutEdidFilename + ".bin");
        }

        if(!anOutInfFilename.isEmpty()) {
            if(anOutInfFilename.isEndsWithIgnoreCase(stCString(".inf"))) {
                anOutInfFilename = anOutInfFilename.subString(0, anOutInfFilename.getLength() - 4);
            }
            genInf(anInputEdid, anOutInfFilename + ".inf");
        }
        st::cout << stostream_text("Press any key to exit...") << st::SYS_PAUSE_EMPTY;
        return 0;
    }

    StString aWelcomeMsg = StString("StMonitorsDump ")
                         + StVersionInfo::getSDKVersionString()
                         + " by Kirill Gavrilov (kirill@sview.ru)\n\n";
    st::cout << st::COLOR_FOR_GREEN << aWelcomeMsg << st::COLOR_FOR_WHITE;

    StSearchMonitors aMonitors;
    aMonitors.init();
    StString aDumpStr = dump(aMonitors);
    aDumpStr += '\n';

    StArrayList<StEDIDParser> anEdids;
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        if(aMonitors[aMonIter].getEdid().isValid()) {
            anEdids.add(aMonitors[aMonIter].getEdid());
        }
    }
    if(anEdids.isEmpty()) {
        StSearchMonitors::listEDID(anEdids);
    }

    for(size_t anIter = 0; anIter < anEdids.size(); ++anIter) {
        StEDIDParser& anEdid = anEdids[anIter];
        aDumpStr += dumpEdid(anEdid);
        if(!anEdid.isValid()) {
            continue;
        }

        StString aSuffix = (anEdids.size() > 1) ? StString(anIter) : StString();
        if(!aPnPIdReplace.isEmpty()) {
            anEdid.setPnPId(aPnPIdReplace);
        }
        if(!anOutEdidFilename.isEmpty()) {
            if(anOutEdidFilename.isEndsWithIgnoreCase(stCString(".bin"))) {
                anOutEdidFilename = anOutEdidFilename.subString(0, anOutEdidFilename.getLength() - 4);
            }
            dumpEdidToFile(anEdid, anOutEdidFilename + aSuffix + ".bin");
        }
        if(!anOutInfFilename.isEmpty()) {
            if(anOutInfFilename.isEndsWithIgnoreCase(stCString(".inf"))) {
                anOutInfFilename = anOutInfFilename.subString(0, anOutInfFilename.getLength() - 4);
            }
            genInf(anEdid, anOutInfFilename + aSuffix + ".inf");
        }
    }
    st::cout << aDumpStr;
    st::cout << stostream_text("\n\n");

    std::ofstream aFileOut;
    bool isTmpFile = false;
    StString aFileName = "stMonitorsDump.htm";
    aFileOut.open(aFileName.toCString());
    if(aFileOut.fail()) {
        aFileName = StProcess::getTempFolder() + "/stMonitorsDump.htm";
        isTmpFile = true;
        aFileOut.open(aFileName.toCString());
        if(aFileOut.fail()) {
            st::cout << st::COLOR_FOR_RED << stostream_text("Couldn't open file \"stMonitorsDump.htm\"!\n") << st::COLOR_FOR_WHITE;
            st::cout << stostream_text("Press any key to exit...") << st::SYS_PAUSE_EMPTY;
            return -1;
        }
    }
    aFileOut << "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head><body><pre>\n";
    aFileOut.write(aDumpStr.toCString(), aDumpStr.getSize());
    aFileOut << "</pre></body></html>\n";
    aFileOut.close();

    st::cout << st::COLOR_FOR_GREEN << stostream_text("Dump stored to file \"stMonitorsDump.htm\"\n") << st::COLOR_FOR_WHITE;

    StProcess::openURL(aFileName);
    st::cout << stostream_text("Press any key to exit...") << st::SYS_PAUSE_EMPTY;
    if(isTmpFile) {
        StFileNode::removeFile(aFileName);
    }
    return 0;
}
