/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StExifTags.h>

namespace StExifTags {

// reference: Exif 2.2
const StExifTag IMAGE_TAGS[] = {

{0x000B, stCString("Exif.Image.ProcessingSoftware")},
{0x00FE, stCString("Exif.Image.NewSubfileType")},
{0x00FF, stCString("Exif.Image.SubfileType")},
{0x0100, stCString("Exif.Image.ImageWidth")},
{0x0101, stCString("Exif.Image.ImageLength")},
{0x0102, stCString("Exif.Image.BitsPerSample")},
{0x0103, stCString("Exif.Image.Compression")},
{0x0106, stCString("Exif.Image.PhotometricInterpretation")},
{0x0107, stCString("Exif.Image.Threshholding")},
{0x0108, stCString("Exif.Image.CellWidth")},
{0x0109, stCString("Exif.Image.CellLength")},
{0x010A, stCString("Exif.Image.FillOrder")},
{0x010D, stCString("Exif.Image.DocumentName")},
{0x010E, stCString("Exif.Image.ImageDescription")},
{0x010F, stCString("Exif.Image.Make")},
{0x0110, stCString("Exif.Image.Model")},
{0x0111, stCString("Exif.Image.StripOffsets")},
{0x0112, stCString("Exif.Image.Orientation")},
{0x0115, stCString("Exif.Image.SamplesPerPixel")},
{0x0116, stCString("Exif.Image.RowsPerStrip")},
{0x0117, stCString("Exif.Image.StripByteCounts")},
{0x011A, stCString("Exif.Image.XResolution")},
{0x011B, stCString("Exif.Image.YResolution")},
{0x011C, stCString("Exif.Image.PlanarConfiguration")},
{0x0122, stCString("Exif.Image.GrayResponseUnit")},
{0x0123, stCString("Exif.Image.GrayResponseCurve")},
{0x0124, stCString("Exif.Image.T4Options")},
{0x0125, stCString("Exif.Image.T6Options")},
{0x0128, stCString("Exif.Image.ResolutionUnit")},
{0x012D, stCString("Exif.Image.TransferFunction")},
{0x0131, stCString("Exif.Image.Software")},
{0x0132, stCString("Exif.Image.DateTime")},
{0x013B, stCString("Exif.Image.Artist")},
{0x013C, stCString("Exif.Image.HostComputer")},
{0x013D, stCString("Exif.Image.Predictor")},
{0x013E, stCString("Exif.Image.WhitePoint")},
{0x013F, stCString("Exif.Image.PrimaryChromaticities")},
{0x0140, stCString("Exif.Image.ColorMap")},
{0x0141, stCString("Exif.Image.HalftoneHints")},
{0x0142, stCString("Exif.Image.TileWidth")},
{0x0143, stCString("Exif.Image.TileLength")},
{0x0144, stCString("Exif.Image.TileOffsets")},
{0x0145, stCString("Exif.Image.TileByteCounts")},
{0x014A, stCString("Exif.Image.SubIFDs")},
{0x014C, stCString("Exif.Image.InkSet")},
{0x014D, stCString("Exif.Image.InkNames")},
{0x014E, stCString("Exif.Image.NumberOfInks")},
{0x0150, stCString("Exif.Image.DotRange")},
{0x0151, stCString("Exif.Image.TargetPrinter")},
{0x0152, stCString("Exif.Image.ExtraSamples")},
{0x0153, stCString("Exif.Image.SampleFormat")},
{0x0154, stCString("Exif.Image.SMinSampleValue")},
{0x0155, stCString("Exif.Image.SMaxSampleValue")},
{0x0156, stCString("Exif.Image.TransferRange")},
{0x0157, stCString("Exif.Image.ClipPath")},
{0x0158, stCString("Exif.Image.XClipPathUnits")},
{0x0159, stCString("Exif.Image.YClipPathUnits")},
{0x015A, stCString("Exif.Image.Indexed")},
{0x015B, stCString("Exif.Image.JPEGTables")},
{0x015F, stCString("Exif.Image.OPIProxy")},
{0x0200, stCString("Exif.Image.JPEGProc")},
{0x0201, stCString("Exif.Image.JPEGInterchangeFormat")},
{0x0202, stCString("Exif.Image.JPEGInterchangeFormatLength")},
{0x0203, stCString("Exif.Image.JPEGRestartInterval")},
{0x0205, stCString("Exif.Image.JPEGLosslessPredictors")},
{0x0206, stCString("Exif.Image.JPEGPointTransforms")},
{0x0207, stCString("Exif.Image.JPEGQTables")},
{0x0208, stCString("Exif.Image.JPEGDCTables")},
{0x0209, stCString("Exif.Image.JPEGACTables")},
{0x0211, stCString("Exif.Image.YCbCrCoefficients")},
{0x0212, stCString("Exif.Image.YCbCrSubSampling")},
{0x0213, stCString("Exif.Image.YCbCrPositioning")},
{0x0214, stCString("Exif.Image.ReferenceBlackWhite")},
{0x02BC, stCString("Exif.Image.XMLPacket")},

{0x4746, stCString("Exif.Image.Rating")},
{0x4749, stCString("Exif.Image.RatingPercent")},

{0x800D, stCString("Exif.Image.ImageID")},
{0x828D, stCString("Exif.Image.CFARepeatPatternDim")},
{0x828E, stCString("Exif.Image.CFAPattern")},
{0x828F, stCString("Exif.Image.BatteryLevel")},
{0x8298, stCString("Exif.Image.Copyright")},
{0x829A, stCString("Exif.Image.ExposureTime")},
{0x829D, stCString("Exif.Image.FNumber")},
{0x83BB, stCString("Exif.Image.IPTCNAA")},
{0x8649, stCString("Exif.Image.ImageResources")},
{0x8769, stCString("Exif.Image.ExifTag")},
{0x8773, stCString("Exif.Image.InterColorProfile")},
{0x8822, stCString("Exif.Image.ExposureProgram")},
{0x8824, stCString("Exif.Image.SpectralSensitivity")},
{0x8825, stCString("Exif.Image.GPSTag")},
{0x8827, stCString("Exif.Image.ISOSpeedRatings")},
{0x8828, stCString("Exif.Image.OECF")},
{0x8829, stCString("Exif.Image.Interlace")},
{0x882A, stCString("Exif.Image.TimeZoneOffset")},
{0x882B, stCString("Exif.Image.SelfTimerMode")},
{0x9003, stCString("Exif.Image.DateTimeOriginal")},
{0x9102, stCString("Exif.Image.CompressedBitsPerPixel")},
{0x9201, stCString("Exif.Image.ShutterSpeedValue")},
{0x9202, stCString("Exif.Image.ApertureValue")},
{0x9203, stCString("Exif.Image.BrightnessValue")},
{0x9204, stCString("Exif.Image.ExposureBiasValue")},
{0x9205, stCString("Exif.Image.MaxApertureValue")},
{0x9206, stCString("Exif.Image.SubjectDistance")},
{0x9207, stCString("Exif.Image.MeteringMode")},
{0x9208, stCString("Exif.Image.LightSource")},
{0x9209, stCString("Exif.Image.Flash")},
{0x920A, stCString("Exif.Image.FocalLength")},
{0x920B, stCString("Exif.Image.FlashEnergy")},
{0x920C, stCString("Exif.Image.SpatialFrequencyResponse")},
{0x920D, stCString("Exif.Image.Noise")},
{0x920E, stCString("Exif.Image.FocalPlaneXResolution")},
{0x920F, stCString("Exif.Image.FocalPlaneYResolution")},
{0x9210, stCString("Exif.Image.FocalPlaneResolutionUnit")},
{0x9211, stCString("Exif.Image.ImageNumber")},
{0x9212, stCString("Exif.Image.SecurityClassification")},
{0x9213, stCString("Exif.Image.ImageHistory")},
{0x9214, stCString("Exif.Image.SubjectLocation")},
{0x9215, stCString("Exif.Image.ExposureIndex")},
{0x9216, stCString("Exif.Image.TIFFEPStandardID")},
{0x9217, stCString("Exif.Image.SensingMethod")},

// in UCS2
{0x9C9B, stCString("Exif.Image.XPTitle")},
{0x9C9C, stCString("Exif.Image.XPComment")},
{0x9C9D, stCString("Exif.Image.XPAuthor")},
{0x9C9E, stCString("Exif.Image.XPKeywords")},
{0x9C9F, stCString("Exif.Image.XPSubject")},

{0xC4A5, stCString("Exif.Image.PrintImageMatching")},
{0xC612, stCString("Exif.Image.DNGVersion")},
{0xC613, stCString("Exif.Image.DNGBackwardVersion")},
{0xC614, stCString("Exif.Image.UniqueCameraModel")},
{0xC615, stCString("Exif.Image.LocalizedCameraModel")},
{0xC616, stCString("Exif.Image.CFAPlaneColor")},
{0xC617, stCString("Exif.Image.CFALayout")},
{0xC618, stCString("Exif.Image.LinearizationTable")},
{0xC619, stCString("Exif.Image.BlackLevelRepeatDim")},
{0xC61A, stCString("Exif.Image.BlackLevel")},
{0xC61B, stCString("Exif.Image.BlackLevelDeltaH")},
{0xC61C, stCString("Exif.Image.BlackLevelDeltaV")},
{0xC61D, stCString("Exif.Image.WhiteLevel")},
{0xC61E, stCString("Exif.Image.DefaultScale")},
{0xC61F, stCString("Exif.Image.DefaultCropOrigin")},
{0xC620, stCString("Exif.Image.DefaultCropSize")},
{0xC621, stCString("Exif.Image.ColorMatrix1")},
{0xC622, stCString("Exif.Image.ColorMatrix2")},
{0xC623, stCString("Exif.Image.CameraCalibration1")},
{0xC624, stCString("Exif.Image.CameraCalibration2")},
{0xC625, stCString("Exif.Image.ReductionMatrix1")},
{0xC626, stCString("Exif.Image.ReductionMatrix2")},
{0xC627, stCString("Exif.Image.AnalogBalance")},
{0xC628, stCString("Exif.Image.AsShotNeutral")},
{0xC629, stCString("Exif.Image.AsShotWhiteXY")},
{0xC62A, stCString("Exif.Image.BaselineExposure")},
{0xC62B, stCString("Exif.Image.BaselineNoise")},
{0xC62C, stCString("Exif.Image.BaselineSharpness")},
{0xC62D, stCString("Exif.Image.BayerGreenSplit")},
{0xC62E, stCString("Exif.Image.LinearResponseLimit")},
{0xC62F, stCString("Exif.Image.CameraSerialNumber")},
{0xC630, stCString("Exif.Image.LensInfo")},
{0xC631, stCString("Exif.Image.ChromaBlurRadius")},
{0xC632, stCString("Exif.Image.AntiAliasStrength")},
{0xC633, stCString("Exif.Image.ShadowScale")},
{0xC634, stCString("Exif.Image.DNGPrivateData")},
{0xC635, stCString("Exif.Image.MakerNoteSafety")},
{0xC65A, stCString("Exif.Image.CalibrationIlluminant1")},
{0xC65B, stCString("Exif.Image.CalibrationIlluminant2")},
{0xC65C, stCString("Exif.Image.BestQualityScale")},
{0xC65D, stCString("Exif.Image.RawDataUniqueID")},
{0xC68B, stCString("Exif.Image.OriginalRawFileName")},
{0xC68C, stCString("Exif.Image.OriginalRawFileData")},
{0xC68D, stCString("Exif.Image.ActiveArea")},
{0xC68E, stCString("Exif.Image.MaskedAreas")},
{0xC68F, stCString("Exif.Image.AsShotICCProfile")},
{0xC690, stCString("Exif.Image.AsShotPreProfileMatrix")},
{0xC691, stCString("Exif.Image.CurrentICCProfile")},
{0xC692, stCString("Exif.Image.CurrentPreProfileMatrix")},
{0xC6BF, stCString("Exif.Image.ColorimetricReference")},
{0xC6F3, stCString("Exif.Image.CameraCalibrationSignature")},
{0xC6F4, stCString("Exif.Image.ProfileCalibrationSignature")},
{0xC6F6, stCString("Exif.Image.AsShotProfileName")},
{0xC6F7, stCString("Exif.Image.NoiseReductionApplied")},
{0xC6F8, stCString("Exif.Image.ProfileName")},
{0xC6F9, stCString("Exif.Image.ProfileHueSatMapDims")},
{0xC6FA, stCString("Exif.Image.ProfileHueSatMapData1")},
{0xC6FB, stCString("Exif.Image.ProfileHueSatMapData2")},
{0xC6FC, stCString("Exif.Image.ProfileToneCurve")},
{0xC6FD, stCString("Exif.Image.ProfileEmbedPolicy")},
{0xC6fE, stCString("Exif.Image.ProfileCopyright")},
{0xC714, stCString("Exif.Image.ForwardMatrix1")},
{0xC715, stCString("Exif.Image.ForwardMatrix2")},
{0xC716, stCString("Exif.Image.PreviewApplicationName")},
{0xC717, stCString("Exif.Image.PreviewApplicationVersion")},
{0xC718, stCString("Exif.Image.PreviewSettingsName")},
{0xC719, stCString("Exif.Image.PreviewSettingsDigest")},
{0xC71A, stCString("Exif.Image.PreviewColorSpace")},
{0xC71B, stCString("Exif.Image.PreviewDateTime")},
{0xC71C, stCString("Exif.Image.RawImageDigest")},
{0xC71D, stCString("Exif.Image.OriginalRawFileDigest")},
{0xC71E, stCString("Exif.Image.SubTileBlockSize")},
{0xC71F, stCString("Exif.Image.RowInterleaveFactor")},
{0xC725, stCString("Exif.Image.ProfileLookTableDims")},
{0xC726, stCString("Exif.Image.ProfileLookTableData")},
{0xC740, stCString("Exif.Image.OpcodeList1")},
{0xC741, stCString("Exif.Image.OpcodeList2")},
{0xC74E, stCString("Exif.Image.OpcodeList3")},
{0xC761, stCString("Exif.Image.NoiseProfile")},

{0x829A, stCString("Exif.Photo.ExposureTime")},
{0x829D, stCString("Exif.Photo.FNumber")},
{0x8822, stCString("Exif.Photo.ExposureProgram")},
{0x8824, stCString("Exif.Photo.SpectralSensitivity")},
{0x8827, stCString("Exif.Photo.ISOSpeedRatings")},
{0x8828, stCString("Exif.Photo.OECF")},
{0x8830, stCString("Exif.Photo.SensitivityType")},
{0x8831, stCString("Exif.Photo.StandardOutputSensitivity")},
{0x8832, stCString("Exif.Photo.RecommendedExposureIndex")},
{0x8833, stCString("Exif.Photo.ISOSpeed")},
{0x8834, stCString("Exif.Photo.ISOSpeedLatitudeyyy")},
{0x8835, stCString("Exif.Photo.ISOSpeedLatitudezzz")},
{0x9000, stCString("Exif.Photo.ExifVersion")},
{0x9003, stCString("Exif.Photo.DateTimeOriginal")},
{0x9004, stCString("Exif.Photo.DateTimeDigitized")},
{0x9101, stCString("Exif.Photo.ComponentsConfiguration")},
{0x9102, stCString("Exif.Photo.CompressedBitsPerPixel")},
{0x9201, stCString("Exif.Photo.ShutterSpeedValue")},
{0x9202, stCString("Exif.Photo.ApertureValue")},
{0x9203, stCString("Exif.Photo.BrightnessValue")},
{0x9204, stCString("Exif.Photo.ExposureBiasValue")},
{0x9205, stCString("Exif.Photo.MaxApertureValue")},
{0x9206, stCString("Exif.Photo.SubjectDistance")},
{0x9207, stCString("Exif.Photo.MeteringMode")},
{0x9208, stCString("Exif.Photo.LightSource")},
{0x9209, stCString("Exif.Photo.Flash")},
{0x920A, stCString("Exif.Photo.FocalLength")},
{0x9214, stCString("Exif.Photo.SubjectArea")},
{0x927C, stCString("Exif.Photo.MakerNote")},
{0x9286, stCString("Exif.Photo.UserComment")},
{0x9290, stCString("Exif.Photo.SubSecTime")},
{0x9291, stCString("Exif.Photo.SubSecTimeOriginal")},
{0x9292, stCString("Exif.Photo.SubSecTimeDigitized")},
{0xA000, stCString("Exif.Photo.FlashpixVersion")},
{0xA001, stCString("Exif.Photo.ColorSpace")},
{0xA002, stCString("Exif.Photo.PixelXDimension")},
{0xA003, stCString("Exif.Photo.PixelYDimension")},
{0xA004, stCString("Exif.Photo.RelatedSoundFile")},
{0xA005, stCString("Exif.Photo.InteroperabilityTag")},
{0xA20B, stCString("Exif.Photo.FlashEnergy")},
{0xA20C, stCString("Exif.Photo.SpatialFrequencyResponse")},
{0xA20E, stCString("Exif.Photo.FocalPlaneXResolution")},
{0xA20F, stCString("Exif.Photo.FocalPlaneYResolution")},
{0xA210, stCString("Exif.Photo.FocalPlaneResolutionUnit")},
{0xA214, stCString("Exif.Photo.SubjectLocation")},
{0xA215, stCString("Exif.Photo.ExposureIndex")},
{0xA217, stCString("Exif.Photo.SensingMethod")},
{0xA300, stCString("Exif.Photo.FileSource")},
{0xA301, stCString("Exif.Photo.SceneType")},
{0xA302, stCString("Exif.Photo.CFAPattern")},
{0xA401, stCString("Exif.Photo.CustomRendered")},
{0xA402, stCString("Exif.Photo.ExposureMode")},
{0xA403, stCString("Exif.Photo.WhiteBalance")},
{0xA404, stCString("Exif.Photo.DigitalZoomRatio")},
{0xA405, stCString("Exif.Photo.FocalLengthIn35mmFilm")},
{0xA406, stCString("Exif.Photo.SceneCaptureType")},
{0xA407, stCString("Exif.Photo.GainControl")},
{0xA408, stCString("Exif.Photo.Contrast")},
{0xA409, stCString("Exif.Photo.Saturation")},
{0xA40A, stCString("Exif.Photo.Sharpness")},
{0xA40B, stCString("Exif.Photo.DeviceSettingDescription")},
{0xA40C, stCString("Exif.Photo.SubjectDistanceRange")},
{0xA420, stCString("Exif.Photo.ImageUniqueID")},
{0xA430, stCString("Exif.Photo.CameraOwnerName")},
{0xA431, stCString("Exif.Photo.BodySerialNumber")},
{0xA432, stCString("Exif.Photo.LensSpecification")},
{0xA433, stCString("Exif.Photo.LensMake")},
{0xA434, stCString("Exif.Photo.LensModel")},
{0xA435, stCString("Exif.Photo.LensSerialNumber")},

{0xFFFF, stCString("")}

};

// reference: CIPA DC-007-2009
// http://www.cipa.jp/std/documents/e/DC-007_E.pdf
const StExifTag MPO_TAGS[] = {

{0xB000, stCString("Exif.MP.MPFVersion")},

// MP Index IFD
{0xB001, stCString("Exif.MP.NumberOfImages")},
{0xB002, stCString("Exif.MP.MPEntry")},
{0xB003, stCString("Exif.MP.ImageUIDList")},
{0xB004, stCString("Exif.MP.TotalFrames")},

// MP Attribute IFD
{0xB101, stCString("Exif.MP.MPIndividualNum")},
{0xB201, stCString("Exif.MP.PanOrientation")},
{0xB202, stCString("Exif.MP.PanOverlap_H")},
{0xB203, stCString("Exif.MP.PanOverlap_V")},
{0xB204, stCString("Exif.MP.BaseViewpointNum")},
{0xB205, stCString("Exif.MP.ConvergenceAngle")},
{0xB206, stCString("Exif.MP.BaselineLength")},
{0xB207, stCString("Exif.MP.VerticalDivergence")},
{0xB208, stCString("Exif.MP.AxisDistance_X")},
{0xB209, stCString("Exif.MP.AxisDistance_Y")},
{0xB20A, stCString("Exif.MP.AxisDistance_Z")},
{0xB20B, stCString("Exif.MP.YawAngle")},
{0xB20C, stCString("Exif.MP.PitchAngle")},
{0xB20D, stCString("Exif.MP.RollAngle")},
{0xB20A, stCString("Exif.MP.AxisDistance_Z")},

{0xFFFF, stCString("")}

};

// reference: http://www.exiv2.org/tags-fujifilm.html
const StExifTag FUJI_TAGS[] = {

{0x0000, stCString("Exif.Fujifilm.Version")},
{0x0010, stCString("Exif.Fujifilm.SerialNumber")},
{0x1000, stCString("Exif.Fujifilm.Quality")},
{0x1001, stCString("Exif.Fujifilm.Sharpness")},
{0x1002, stCString("Exif.Fujifilm.WhiteBalance")},
{0x1003, stCString("Exif.Fujifilm.Color")},
{0x1004, stCString("Exif.Fujifilm.Tone")},
{0x1010, stCString("Exif.Fujifilm.FlashMode")},
{0x1011, stCString("Exif.Fujifilm.FlashStrength")},
{0x1020, stCString("Exif.Fujifilm.Macro")},
{0x1021, stCString("Exif.Fujifilm.FocusMode")},
{0x1030, stCString("Exif.Fujifilm.SlowSync")},
{0x1031, stCString("Exif.Fujifilm.PictureMode")},
{0x1100, stCString("Exif.Fujifilm.Continuous")},
{0x1101, stCString("Exif.Fujifilm.SequenceNumber")},
{0x1210, stCString("Exif.Fujifilm.FinePixColor")},
{0x1300, stCString("Exif.Fujifilm.BlurWarning")},
{0x1301, stCString("Exif.Fujifilm.FocusWarning")},
{0x1302, stCString("Exif.Fujifilm.ExposureWarning")},
{0x1400, stCString("Exif.Fujifilm.DynamicRange")},
{0x1401, stCString("Exif.Fujifilm.FilmMode")},
{0x1402, stCString("Exif.Fujifilm.DynamicRangeSetting")},
{0x1403, stCString("Exif.Fujifilm.DevelopmentDynamicRange")},
{0x1404, stCString("Exif.Fujifilm.MinFocalLength")},
{0x1405, stCString("Exif.Fujifilm.MaxFocalLength")},
{0x1406, stCString("Exif.Fujifilm.MaxApertureAtMinFocal")},
{0x1407, stCString("Exif.Fujifilm.MaxApertureAtMaxFocal")},
{0x8000, stCString("Exif.Fujifilm.FileSource")},
{0x8002, stCString("Exif.Fujifilm.OrderNumber")},
{0x8003, stCString("Exif.Fujifilm.FrameNumber")},

{0xB211, stCString("Exif.Fujifilm.Parallax")},

{0xFFFF, stCString("")}

};

// reference: http://www.exiv2.org/tags-canon.html
const StExifTag CANON_TAGS[] = {

{0x0001, stCString("Exif.Canon.CameraSettings")},
{0x0002, stCString("Exif.Canon.FocalLength")},
{0x0004, stCString("Exif.Canon.ShotInfo")},
{0x0005, stCString("Exif.Canon.Panorama")},
{0x0006, stCString("Exif.Canon.ImageType")},
{0x0007, stCString("Exif.Canon.FirmwareVersion")},
{0x0008, stCString("Exif.Canon.FileNumber")},
{0x0009, stCString("Exif.Canon.OwnerName")},
{0x000C, stCString("Exif.Canon.SerialNumber")},
{0x000D, stCString("Exif.Canon.CameraInfo")},
{0x000F, stCString("Exif.Canon.CustomFunctions")},
{0x0010, stCString("Exif.Canon.ModelID")},
{0x0012, stCString("Exif.Canon.PictureInfo")},
{0x0013, stCString("Exif.Canon.ThumbnailImageValidArea")},
{0x0015, stCString("Exif.Canon.SerialNumberFormat")},
{0x001A, stCString("Exif.Canon.SuperMacro")},
{0x0026, stCString("Exif.Canon.AFInfo")},
{0x0083, stCString("Exif.Canon.OriginalDecisionDataOffset")},
{0x00A4, stCString("Exif.Canon.WhiteBalanceTable")},
{0x0095, stCString("Exif.Canon.LensModel")},
{0x0096, stCString("Exif.Canon.InternalSerialNumber")},
{0x0097, stCString("Exif.Canon.DustRemovalData")},
{0x0099, stCString("Exif.Canon.CustomFunctions")},
{0x00A0, stCString("Exif.Canon.ProcessingInfo")},
{0x00AA, stCString("Exif.Canon.MeasuredColor")},
{0x00B4, stCString("Exif.Canon.ColorSpace")},
{0x00D0, stCString("Exif.Canon.VRDOffset")},
{0x00E0, stCString("Exif.Canon.SensorInfo")},
{0x4001, stCString("Exif.Canon.ColorData")},

{0xFFFF, stCString("")}

};

// reference: http://www.exiv2.org/tags-olympus.html
const StExifTag OLYMP_TAGS[] = {

{0x0100, stCString("Exif.Olympus.ThumbnailImage")},
{0x0104, stCString("Exif.Olympus.BodyFirmwareVersion")},
{0x0200, stCString("Exif.Olympus.SpecialMode")},
{0x0201, stCString("Exif.Olympus.Quality")},
{0x0202, stCString("Exif.Olympus.Macro")},
{0x0203, stCString("Exif.Olympus.BWMode")},
{0x0204, stCString("Exif.Olympus.DigitalZoom")},
{0x0205, stCString("Exif.Olympus.FocalPlaneDiagonal")},
{0x0206, stCString("Exif.Olympus.LensDistortionParams")},
{0x0207, stCString("Exif.Olympus.CameraType")},
{0x0208, stCString("Exif.Olympus.PictureInfo")},
{0x0209, stCString("Exif.Olympus.CameraID")},
{0x020B, stCString("Exif.Olympus.ImageWidth")},
{0x020C, stCString("Exif.Olympus.ImageHeight")},
{0x020D, stCString("Exif.Olympus.Software")},
{0x0280, stCString("Exif.Olympus.PreviewImage")},
{0x0300, stCString("Exif.Olympus.PreCaptureFrames")},
{0x0301, stCString("Exif.Olympus.WhiteBoard")},
{0x0302, stCString("Exif.Olympus.OneTouchWB")},
{0x0303, stCString("Exif.Olympus.WhiteBalanceBracket")},
{0x0304, stCString("Exif.Olympus.WhiteBalanceBias")},
//{0x0403, stCString("Exif.Olympus.SceneMode")},
{0x0404, stCString("Exif.Olympus.Firmware")},
{0x0E00, stCString("Exif.Olympus.PrintIM")},
{0x0F00, stCString("Exif.Olympus.DataDump1")},
{0x0F01, stCString("Exif.Olympus.DataDump2")},
{0x1000, stCString("Exif.Olympus.ShutterSpeed")},
{0x1001, stCString("Exif.Olympus.ISOSpeed")},
{0x1002, stCString("Exif.Olympus.ApertureValue")},
{0x1003, stCString("Exif.Olympus.Brightness")},
{0x1004, stCString("Exif.Olympus.FlashMode")},
{0x1005, stCString("Exif.Olympus.FlashDevice")},
{0x1006, stCString("Exif.Olympus.Bracket")},
{0x1007, stCString("Exif.Olympus.SensorTemperature")},
{0x1008, stCString("Exif.Olympus.LensTemperature")},
{0x1009, stCString("Exif.Olympus.LightCondition")},
{0x100A, stCString("Exif.Olympus.FocusRange")},
{0x100B, stCString("Exif.Olympus.FocusMode")},
{0x100C, stCString("Exif.Olympus.FocusDistance")},
{0x100D, stCString("Exif.Olympus.Zoom")},
{0x100E, stCString("Exif.Olympus.MacroFocus")},
{0x100F, stCString("Exif.Olympus.SharpnessFactor")},
{0x1010, stCString("Exif.Olympus.FlashChargeLevel")},
{0x1011, stCString("Exif.Olympus.ColorMatrix")},
{0x1012, stCString("Exif.Olympus.BlackLevel")},
{0x1015, stCString("Exif.Olympus.WhiteBalance")},
{0x1017, stCString("Exif.Olympus.RedBalance")},
{0x1018, stCString("Exif.Olympus.BlueBalance")},
{0x1019, stCString("Exif.Olympus.ColorMatrixNumber")},
{0x101A, stCString("Exif.Olympus.SerialNumber2")},
{0x1023, stCString("Exif.Olympus.FlashBias")},
{0x1026, stCString("Exif.Olympus.ExternalFlashBounce")},
{0x1027, stCString("Exif.Olympus.ExternalFlashZoom")},
{0x1028, stCString("Exif.Olympus.ExternalFlashMode")},
{0x1029, stCString("Exif.Olympus.Contrast")},
{0x102A, stCString("Exif.Olympus.SharpnessFactor")},
{0x102B, stCString("Exif.Olympus.ColorControl")},
{0x102C, stCString("Exif.Olympus.ValidBits")},
{0x102D, stCString("Exif.Olympus.CoringFilter")},
{0x102E, stCString("Exif.Olympus.ImageWidth")},
{0x102F, stCString("Exif.Olympus.ImageHeight")},
{0x1034, stCString("Exif.Olympus.CompressionRatio")},
{0x1035, stCString("Exif.Olympus.Thumbnail")},
{0x1036, stCString("Exif.Olympus.ThumbnailOffset")},
{0x1037, stCString("Exif.Olympus.ThumbnailLength")},
{0x1039, stCString("Exif.Olympus.CCDScanMode")},
{0x103A, stCString("Exif.Olympus.NoiseReduction")},
{0x103B, stCString("Exif.Olympus.InfinityLensStep")},
{0x103C, stCString("Exif.Olympus.NearLensStep")},
{0x2010, stCString("Exif.Olympus.Equipment")},
{0x2020, stCString("Exif.Olympus.CameraSettings")},
{0x2030, stCString("Exif.Olympus.RawDevelopment")},
{0x2031, stCString("Exif.Olympus.RawDevelopment2")},
{0x2040, stCString("Exif.Olympus.ImageProcessing")},
{0x2050, stCString("Exif.Olympus.FocusInfo")},
{0x3000, stCString("Exif.Olympus.RawInfo")},

{0xFFFF, stCString("")}

};

} // namespace

StExifTagsMap::StExifTagsMap() {
    for(const StExifTag* anIter = StExifTags::IMAGE_TAGS; anIter->Tag != 0xFFFF; ++anIter) {
        myImageTags[anIter->Tag] = anIter;
    }
    for(const StExifTag* anIter = StExifTags::MPO_TAGS; anIter->Tag != 0xFFFF; ++anIter) {
        myMpoTags[anIter->Tag] = anIter;
    }
    for(const StExifTag* anIter = StExifTags::FUJI_TAGS; anIter->Tag != 0xFFFF; ++anIter) {
        myFujiTags[anIter->Tag] = anIter;
    }
    for(const StExifTag* anIter = StExifTags::CANON_TAGS; anIter->Tag != 0xFFFF; ++anIter) {
        myCanonTags[anIter->Tag] = anIter;
    }
    for(const StExifTag* anIter = StExifTags::OLYMP_TAGS; anIter->Tag != 0xFFFF; ++anIter) {
        myOlympTags[anIter->Tag] = anIter;
    }
}

StExifTagsMap::~StExifTagsMap() {
    //
}

const StExifTag* StExifTagsMap::findImageTag(const uint16_t theTag) const {
    const std::map<uint16_t, const StExifTag*>::const_iterator anIter = myImageTags.find(theTag);
    return (anIter != myImageTags.end()) ? anIter->second : NULL;
}

const StExifTag* StExifTagsMap::findMpoTag(const uint16_t theTag) const {
    const std::map<uint16_t, const StExifTag*>::const_iterator anIter = myMpoTags.find(theTag);
    return (anIter != myMpoTags.end()) ? anIter->second : NULL;
}

const StExifTag* StExifTagsMap::findFujiTag(const uint16_t theTag) const {
    const std::map<uint16_t, const StExifTag*>::const_iterator anIter = myFujiTags.find(theTag);
    return (anIter != myFujiTags.end()) ? anIter->second : NULL;
}

const StExifTag* StExifTagsMap::findCanonTag(const uint16_t theTag) const {
    const std::map<uint16_t, const StExifTag*>::const_iterator anIter = myCanonTags.find(theTag);
    return (anIter != myCanonTags.end()) ? anIter->second : NULL;
}

const StExifTag* StExifTagsMap::findOlympTag(const uint16_t theTag) const {
    const std::map<uint16_t, const StExifTag*>::const_iterator anIter = myOlympTags.find(theTag);
    return (anIter != myOlympTags.end()) ? anIter->second : NULL;
}
