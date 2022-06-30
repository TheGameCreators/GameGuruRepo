#ifndef BOXER_H
#define BOXER_H

#if defined(BOXER_DLL) && defined(BOXER_BUILD_DLL)
   #error "You may not have both BOXER_DLL and BOXER_BUILD_DLL defined"
#endif
#define BOXERAPI
namespace boxer {

/*!
 * Options for styles to apply to a message box
 */
enum class Style {
   Info,
   Warning,
   Error,
   Question
};

/*!
 * Options for buttons to provide on a message box
 */
enum class Buttons {
   OK,
   OKCancel,
   YesNo,
   Quit,
   YesNoCancel
};

/*!
 * Possible responses from a message box. 'None' signifies that no option was chosen, and 'Error' signifies that an
 * error was encountered while creating the message box.
 */
enum class Selection {
   OK,
   Cancel,
   Yes,
   No,
   Quit,
   NOne,
   Error
};

/*!
 * The default style to apply to a message box
 */
const Style kDefaultStyle = Style::Info;

/*!
 * The default buttons to provide on a message box
 */
const Buttons kDefaultButtons = Buttons::OK;

/*!
 * Blocking call to create a modal message box with the given message, title, style, and buttons
 */
BOXERAPI Selection show(const char *message, const char *title, Style style, Buttons buttons);

/*!
 * Convenience function to call show() with the default buttons
 */
inline Selection show(const char *message, const char *title, Style style) {
   return show(message, title, style, kDefaultButtons);
}

/*!
 * Convenience function to call show() with the default style
 */
inline Selection show(const char *message, const char *title, Buttons buttons) {
   return show(message, title, kDefaultStyle, buttons);
}

/*!
 * Convenience function to call show() with the default style and buttons
 */
inline Selection show(const char *message, const char *title) {
   return show(message, title, kDefaultStyle, kDefaultButtons);
}

} // namespace boxer

#endif

#ifdef AGK_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace boxer {

	namespace {

		UINT getIcon(Style style) {
			switch (style) {
			case Style::Info:
				return MB_ICONINFORMATION;
			case Style::Warning:
				return MB_ICONWARNING;
			case Style::Error:
				return MB_ICONERROR;
			case Style::Question:
				return MB_ICONQUESTION;
			default:
				return MB_ICONINFORMATION;
			}
		}

		UINT getButtons(Buttons buttons) {
			switch (buttons) {
			case Buttons::OK:
			case Buttons::Quit: // There is no 'Quit' button on Windows :(
				return MB_OK;
			case Buttons::OKCancel:
				return MB_OKCANCEL;
			case Buttons::YesNo:
				return MB_YESNO;
			case Buttons::YesNoCancel:
				return MB_YESNOCANCEL;
			default:
				return MB_OK;
			}
		}

		Selection getSelection(int response, Buttons buttons) {
			switch (response) {
			case IDOK:
				return buttons == Buttons::Quit ? Selection::Quit : Selection::OK;
			case IDCANCEL:
				return Selection::Cancel;
			case IDYES:
				return Selection::Yes;
			case IDNO:
				return Selection::No;
			default:
				return Selection::NOne;
			}
		}

	} // namespace

	Selection show(const char *message, const char *title, Style style, Buttons buttons) {
		UINT flags = MB_TASKMODAL;

		flags |= getIcon(style);
		flags |= getButtons(buttons);

//		return getSelection(MessageBoxA(nullptr, message, title, flags), buttons);
		return getSelection(MessageBoxA(g_agkhWnd, message, title, flags), buttons);
		//
	}

} // namespace boxer
#else
#ifdef AGK_MACOS
#import <Cocoa/Cocoa.h>

namespace boxer {

	namespace {

		NSString* const kOkStr = @"OK";
			NSString* const kCancelStr = @"Cancel";
			NSString* const kYesStr = @"Yes";
			NSString* const kNoStr = @"No";
			NSString* const kQuitStr = @"Quit";

			NSAlertStyle getAlertStyle(Style style) {
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
			switch (style) {
			case Style::Info:
				return NSAlertStyleInformational;
			case Style::Warning:
				return NSAlertStyleWarning;
			case Style::Error:
				return NSAlertStyleCritical;
			case Style::Question:
				return NSAlertStyleWarning;
			default:
				return NSAlertStyleInformational;
			}
#else
			switch (style) {
			case Style::Info:
				return NSInformationalAlertStyle;
			case Style::Warning:
				return NSWarningAlertStyle;
			case Style::Error:
				return NSCriticalAlertStyle;
			case Style::Question:
				return NSWarningAlertStyle;
			default:
				return NSInformationalAlertStyle;
			}
#endif
		}

		void setButtons(NSAlert *alert, Buttons buttons) {
			switch (buttons) {
			case Buttons::OK:
				[alert addButtonWithTitle : kOkStr];
				break;
			case Buttons::OKCancel:
				[alert addButtonWithTitle : kOkStr];
				[alert addButtonWithTitle : kCancelStr];
				break;
			case Buttons::YesNo:
				[alert addButtonWithTitle : kYesStr];
				[alert addButtonWithTitle : kNoStr];
				break;
			case Buttons::Quit:
				[alert addButtonWithTitle : kQuitStr];
				break;
			default:
				[alert addButtonWithTitle : kOkStr];
			}
		}

		Selection getSelection(int index, Buttons buttons) {
			switch (buttons) {
			case Buttons::OK:
				return index == NSAlertFirstButtonReturn ? Selection::OK : Selection::NOne;
			case Buttons::OKCancel:
				if (index == NSAlertFirstButtonReturn) {
					return Selection::OK;
				}
				else if (index == NSAlertSecondButtonReturn) {
					return Selection::Cancel;
				}
				else {
					return Selection::NOne;
				}
			case Buttons::YesNo:
				if (index == NSAlertFirstButtonReturn) {
					return Selection::Yes;
				}
				else if (index == NSAlertSecondButtonReturn) {
					return Selection::No;
				}
				else {
					return Selection::NOne;
				}
			case Buttons::Quit:
				return index == NSAlertFirstButtonReturn ? Selection::Quit : Selection::NOne;
			default:
				return Selection::NOne;
			}
		}

	} // namespace

	Selection show(const char *message, const char *title, Style style, Buttons buttons) {
		NSAlert *alert = [[NSAlert alloc] init];

		[alert setMessageText : [NSString stringWithCString : title
			encoding : [NSString defaultCStringEncoding]]];
		[alert setInformativeText : [NSString stringWithCString : message
			encoding : [NSString defaultCStringEncoding]]];

		[alert setAlertStyle : getAlertStyle(style)];
		setButtons(alert, buttons);

		// Force the alert to appear on top of any other windows
		[[alert window] setLevel:NSModalPanelWindowLevel];

		Selection selection = getSelection([alert runModal], buttons);
		[alert release];

		return selection;
	}

} // namespace boxer
#else

#include <gtk/gtk.h>

namespace boxer {

	namespace {

		GtkMessageType getMessageType(Style style) {
			switch (style) {
			case Style::Info:
				return GTK_MESSAGE_INFO;
			case Style::Warning:
				return GTK_MESSAGE_WARNING;
			case Style::Error:
				return GTK_MESSAGE_ERROR;
			case Style::Question:
				return GTK_MESSAGE_QUESTION;
			default:
				return GTK_MESSAGE_INFO;
			}
		}

		GtkButtonsType getButtonsType(Buttons buttons) {
			switch (buttons) {
			case Buttons::OK:
				return GTK_BUTTONS_OK;
			case Buttons::OKCancel:
				return GTK_BUTTONS_OK_CANCEL;
			case Buttons::YesNo:
				return GTK_BUTTONS_YES_NO;
			case Buttons::Quit:
				return GTK_BUTTONS_CLOSE;
			default:
				return GTK_BUTTONS_OK;
			}
		}

		Selection getSelection(gint response) {
			switch (response) {
			case GTK_RESPONSE_OK:
				return Selection::OK;
			case GTK_RESPONSE_CANCEL:
				return Selection::Cancel;
			case GTK_RESPONSE_YES:
				return Selection::Yes;
			case GTK_RESPONSE_NO:
				return Selection::No;
			case GTK_RESPONSE_CLOSE:
				return Selection::Quit;
			default:
				return Selection::NOne;
			}
		}

	} // namespace

	Selection show(const char *message, const char *title, Style style, Buttons buttons) {
		if (!gtk_init_check(0, nullptr)) {
			return Selection::Error;
		}

		// Create a parent window to stop gtk_dialog_run from complaining
		GtkWidget *parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);

		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
			GTK_DIALOG_MODAL,
			getMessageType(style),
			getButtonsType(buttons),
			"%s",
			message);
		gtk_window_set_title(GTK_WINDOW(dialog), title);
		Selection selection = getSelection(gtk_dialog_run(GTK_DIALOG(dialog)));

		gtk_widget_destroy(GTK_WIDGET(dialog));
		gtk_widget_destroy(GTK_WIDGET(parent));
		while (g_main_context_iteration(nullptr, false));

		return selection;
	}

} // namespace boxer
#endif
#endif
