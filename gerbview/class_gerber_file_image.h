/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2010-2016 Jean-Pierre Charras  jp.charras at wanadoo.fr
 * Copyright (C) 1992-2016 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef CLASS_GERBER_FILE_IMAGE_H
#define CLASS_GERBER_FILE_IMAGE_H

#include <vector>
#include <set>

#include <dcode.h>
#include <class_gerber_draw_item.h>
#include <class_aperture_macro.h>
#include <gbr_netlist_metadata.h>

// An useful macro used when reading gerber files;
#define IsNumber( x ) ( ( ( (x) >= '0' ) && ( (x) <='9' ) )   \
                       || ( (x) == '-' ) || ( (x) == '+' )  || ( (x) == '.' ) )
#define IsASCIIDigit(a) (((unsigned)(a)-'0') < 10)
#define IsASCIISpace(a) ((a) == ' ' || (unsigned)(a)-'\t' < 5)

class GERBVIEW_FRAME;
class D_CODE;

/* gerber files have different parameters to define units and how items must be plotted.
 *  some are for the entire file, and other can change along a file.
 *  In Gerber world:
 *  an image is the entire gerber file and its "global" parameters
 *  a layer (that is very different from a board layer) is just a sub set of a file that
 *  have specific parameters
 *  if a Image parameter is set more than once, only the last value is used
 *  Some parameters can change along a file and are not layer specific: they are stored
 *  in GERBER_ITEM items, when instancied.
 *
 *  In GerbView, to handle these parameters, there are 2 classes:
 *  GERBER_FILE_IMAGE : the main class containing most of parameters and data to plot a graphic layer
 *  Some of them can change along the file
 *  There is one GERBER_FILE_IMAGE per file and one graphic layer per file or GERBER_FILE_IMAGE
 *  GerbView does not read and merge 2 gerber file in one graphic layer:
 *  I believe this is not possible due to the constraints in Image parameters.
 *  GERBER_LAYER : containing the subset of parameters that is layer speficic
 *  A GERBER_FILE_IMAGE must include one GERBER_LAYER to define all parameters to plot a file.
 *  But a GERBER_FILE_IMAGE can use more than one GERBER_LAYER.
 */

class GERBER_FILE_IMAGE;
class X2_ATTRIBUTE;
class X2_ATTRIBUTE_FILEFUNCTION;


class GERBER_LAYER
{
    friend class GERBER_FILE_IMAGE;
public:

    // These parameters are layer specfic:
    wxString    m_LayerName;            // Layer name, from LN <name>* command
    bool        m_LayerNegative;        // true = Negative Layer: command LP
    wxRealPoint m_StepForRepeat;        // X and Y offsets for Step and Repeat command
    int         m_XRepeatCount;         // The repeat count on X axis
    int         m_YRepeatCount;         // The repeat count on Y axis
    bool        m_StepForRepeatMetric;  // false = Inches, true = metric
                                        // needed here because repeated
                                        // gerber items can have coordinates
                                        // in different units than step parameters
                                        // and the actual coordinates calculation must handle this

public:
    GERBER_LAYER();
    ~GERBER_LAYER();
private:
    void ResetDefaultValues();
};

/**
 * Class GERBER_FILE_IMAGE
 * holds the Image data and parameters for one gerber file
 * and layer parameters (TODO: move them in GERBER_LAYER class
 */
class GERBER_FILE_IMAGE
{
    D_CODE*            m_Aperture_List[TOOLS_MAX_COUNT];    ///< Dcode (Aperture) List for this layer (max 999)
    bool               m_Exposure;                          ///< whether an aperture macro tool is flashed on or off

    GERBER_LAYER       m_GBRLayerParams; // hold params for the current gerber layer

public:
    DLIST<GERBER_DRAW_ITEM> m_Drawings;                         // linked list of Gerber Items to draw

    bool               m_InUse;                                 // true if this image is currently in use
                                                                // (a file is loaded in it)
    bool               m_IsVisible;                             // true if the draw layer is visible and must be drawn
                                                                // false if it must be not drawn
    COLOR4D            m_PositiveDrawColor;                     // The color used to draw positive items
    wxString           m_FileName;                              // Full File Name for this layer
    wxString           m_ImageName;                             // Image name, from IN <name>* command
    bool               m_IsX2_file;                             // true if a X2 gerber attribute was found in file
    X2_ATTRIBUTE_FILEFUNCTION* m_FileFunction;                  // file function parameters, found in a %TF command
                                                                // or a G04
    wxString           m_MD5_value;                             // MD5 value found in a %TF.MD5 command
    wxString           m_PartString;                            // string found in a %TF.Part command
    int                m_GraphicLayer;                          // Graphic layer Number
    bool               m_ImageNegative;                         // true = Negative image
    bool               m_ImageJustifyXCenter;                   // Image Justify Center on X axis (default = false)
    bool               m_ImageJustifyYCenter;                   // Image Justify Center on Y axis (default = false)
    wxPoint            m_ImageJustifyOffset;                    // Image Justify Offset on XY axis (default = 0,0)
    bool               m_GerbMetric;                            // false = Inches, true = metric
    bool               m_Relative;                              // false = absolute Coord, true = relative Coord
    bool               m_NoTrailingZeros;                       // true: remove tailing zeros.
    bool               m_DecimalFormat;                         // true: use floating point notations for coordinates
                                                                // If true, overrides m_NoTrailingZeros parameter.
    wxPoint            m_ImageOffset;                           // Coord Offset, from IO command
    wxSize             m_FmtScale;                              // Fmt 2.3: m_FmtScale = 3, fmt 3.4: m_FmtScale = 4
    wxSize             m_FmtLen;                                // Nb chars per coord. ex fmt 2.3, m_FmtLen = 5
    int                m_ImageRotation;                         // Image rotation (0, 90, 180, 270 only) in degrees
    double             m_LocalRotation;                         // Local rotation, in degrees, added to m_ImageRotation
                                                                //  Note this value is stored in 0.1 degrees
    wxPoint            m_Offset;                                // Coord Offset, from OF command
    wxRealPoint        m_Scale;                                 // scale (X and Y) of layer.
    bool               m_SwapAxis;                              // false (default) if A = X and B = Y
                                                                // true if A = Y, B = X
    bool               m_MirrorA;                               // true: miror / axe A (X)
    bool               m_MirrorB;                               // true: miror / axe B (Y)
    int                m_Iterpolation;                          // Linear, 90 arc, Circ.
    int                m_Current_Tool;                          // Current Tool (Dcode) number selected
    int                m_Last_Pen_Command;                      // Current or last pen state (0..9, set by Dn option with n <10
    int                m_CommandState;                          // state of gerber analysis command.
    wxPoint            m_CurrentPos;                            // current specified coord for plot
    wxPoint            m_PreviousPos;                           // old current specified coord for plot
    wxPoint            m_IJPos;                                 // IJ coord (for arcs & circles )

    FILE*              m_Current_File;                          // Current file to read
    #define            INCLUDE_FILES_CNT_MAX 10
    FILE*              m_FilesList[INCLUDE_FILES_CNT_MAX + 2];  // Included files list
    int                m_FilesPtr;                              // Stack pointer for files list

    int                m_Selected_Tool;                         // For hightlight: current selected Dcode
    bool               m_Has_DCode;                             // true = DCodes in file
                                                                // (false = no DCode -> separate DCode file
    bool               m_360Arc_enbl;                           // Enbl 360 deg circular interpolation
    bool               m_PolygonFillMode;                       // Enable polygon mode (read coord as a polygon descr)
    int                m_PolygonFillModeState;                  // In polygon mode: 0 = first segm, 1 = next segm

    APERTURE_MACRO_SET m_aperture_macros;                       ///< a collection of APERTURE_MACROS, sorted by name

    GBR_NETLIST_METADATA m_NetAttributeDict;                    // the net attributes set by a %TO.CN, %TO.C and/or %TO.N
                                                                // add object attribute command.
    wxString          m_AperFunction;                           // the aperture function set by a %TA.AperFunction, xxx
                                                                // (stores thre xxx value).

    std::map<wxString, int> m_ComponentsList;                   // list of components
    std::map<wxString, int> m_NetnamesList;                     // list of net names

private:
    wxArrayString      m_messagesList;                          // A list of messages created when reading a file
    int                m_hasNegativeItems;                      // true if the image is negative or has some negative items
                                                                // Used to optimize drawing, because when there are no
                                                                // negative items screen refresh does not need
                                                                // to build an intermediate bitmap specfic to this image
                                                                // -1 = negative items are
                                                                // 0 = no negative items found
                                                                // 1 = have negative items found

public:
    GERBER_FILE_IMAGE( int layer );
    virtual ~GERBER_FILE_IMAGE();

    void Clear_GERBER_FILE_IMAGE();

    /**
     * Read and load a gerber file.
     * @param aFullFileName = the full filename of the Gerber file
     * when the file cannot be loaded
     * Warning and info messages are stored in m_messagesList
     * @return bool if OK, false if the gerber file was not loaded
     */
    bool LoadGerberFile( const wxString& aFullFileName );

    const wxArrayString& GetMessages() const { return m_messagesList; }

    /**
     * @return the count of Dcode tools in used by the image
     */
    int GetDcodesCount();

    virtual void ResetDefaultValues();

    COLOR4D GetPositiveDrawColor() const { return m_PositiveDrawColor; }

    /**
     * Function GetItemsList
     * @return the first GERBER_DRAW_ITEM * item of the items list
     */
    GERBER_DRAW_ITEM * GetItemsList();

    /**
     * Function GetLayerParams
     * @return the current layers params
     */
    GERBER_LAYER& GetLayerParams()
    {
        return m_GBRLayerParams;
    }

    /**
     * Function HasNegativeItems
     * @return true if at least one item must be drawn in background color
     * used to optimize screen refresh (when no items are in background color
     * refresh can be faster)
     */
    bool HasNegativeItems();

    /**
     * Function ClearMessageList
     * Clear the message list
     * Call it before reading a Gerber file
     */
    void    ClearMessageList()
    {
        m_messagesList.Clear();
    }

    /**
     * Function AddMessageToList
     * Add a message to the message list
     */
    void    AddMessageToList( const wxString& aMessage )
    {
        m_messagesList.Add( aMessage );
    }

    /**
     * Function InitToolTable
     */
    void    InitToolTable();

    /**
     * Function ReadXYCoord
     * Returns the current coordinate type pointed to by XnnYnn Text (XnnnnYmmmm)
     */
    wxPoint ReadXYCoord( char*& Text );

    /**
     * Function ReadIJCoord
     * Returns the current coordinate type pointed to by InnJnn Text (InnnnJmmmm)
     * These coordinates are relative, so if coordinate is absent, it's value
     * defaults to 0
     */
    wxPoint ReadIJCoord( char*& Text );

    // functions to read G commands or D commands:
    int     GCodeNumber( char*& Text );
    int     DCodeNumber( char*& Text );

    // functions to execute G commands or D commands:
    bool    Execute_G_Command( char*& text, int G_command );
    bool    Execute_DCODE_Command( char*& text, int D_command );

    /**
     * Function ReadRS274XCommand
     * reads a single RS274X command terminated with a %
     */
    bool ReadRS274XCommand( char *aBuff, char* & text );

    /**
     * Function ExecuteRS274XCommand
     * executes 1 command
     */
    bool ExecuteRS274XCommand( int command, char* aBuff,
                               char*& text );


    /**
     * Function ReadApertureMacro
     * reads in an aperture macro and saves it in m_aperture_macros.
     * @param aBuff a character buffer at least GERBER_BUFZ long that can be
     *          used to read successive lines from the gerber file.
     * @param text A reference to a character pointer which gives the initial
     *              text to read from.
     * @param gerber_file Which file to read from for continuation.
     * @return bool - true if a macro was read in successfully, else false.
     */
    bool ReadApertureMacro( char *aBuff, char* & text,
                            FILE * gerber_file );


    /**
     * Function GetDCODE
     * returns a pointer to the D_CODE within this GERBER for the given
     * \a aDCODE.
     * @param aDCODE The numeric value of the D_CODE to look up.
     * @param aCreateIfNoExist If true, then create the D_CODE if it does not
     *                         exist in list.
     * @return D_CODE* - the one implied by the given \a aDCODE, or NULL
     *            if the requested \a aDCODE is out of range.
     */
    D_CODE*         GetDCODE( int aDCODE, bool aCreateIfNoExist = true );

    /**
     * Function FindApertureMacro
     * looks up a previously read in aperture macro.
     * @param aLookup A dummy APERTURE_MACRO with [only] the name field set.
     * @return APERTURE_MACRO* - the one with a matching name, or NULL if
     *  not found.
     */
    APERTURE_MACRO* FindApertureMacro( const APERTURE_MACRO& aLookup );

    /**
     * Function StepAndRepeatItem
     * Gerber format has a command Step an Repeat
     * This function must be called when reading a gerber file and
     * after creating a new gerber item that must be repeated
     * (i.e when m_XRepeatCount or m_YRepeatCount are > 1)
     * @param aItem = the item to repeat
     */
    void            StepAndRepeatItem( const GERBER_DRAW_ITEM& aItem );

    /**
     * Function DisplayImageInfo
     * has knowledge about the frame and how and where to put status information
     * about this object into the frame's message panel.
     * Display info about Image Parameters.
     * @param aMainFrame = the GERBVIEW_FRAME to display messages
     */
    void DisplayImageInfo( GERBVIEW_FRAME* aMainFrame );

    /**
     * Function RemoveAttribute.
     * Called when a %TD command is found the Gerber file
     * @param aAttribute is the X2_ATTRIBUTE which stores the full command
     * Remove the attribute specified by the %TD command.
     * is no attribute, all current attributes specified by the %TO and the %TA
     * commands are cleared.
     * if a attribute name is specified (for instance %TD.CN*%) is specified,
     * only this attribute is cleared
     */
    void RemoveAttribute( X2_ATTRIBUTE& aAttribute );
};

#endif  // ifndef CLASS_GERBER_FILE_IMAGE_H
