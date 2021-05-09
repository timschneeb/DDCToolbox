#ifndef OSX_HTML_SIZING_PATCH_H
#define OSX_HTML_SIZING_PATCH_H

#include <QRegularExpression>
#include <QString>

#define OSX_HTML_POINT_SIZE_OFFSET 4

class OSXHtmlSizingPatch {
public:
    static QString patchTextSize(QString html){
#ifdef __APPLE__
        // OSX HTML render DPI size fix
        QList<double> sizes;
        QRegularExpression re("font-size:(\\d+\\.?\\d*)pt;");
        QRegularExpressionMatchIterator i = re.globalMatch(html);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            if (match.hasMatch()) {
                float f = match.captured(1).toFloat();
                if(!sizes.contains(f))
                    sizes.append(f);
            }
        }

        for(auto size : sizes){
            html.replace(QString("font-size:%1pt;").arg(size),
                         QString("font-size:%1pt;").arg(size + OSX_HTML_POINT_SIZE_OFFSET));
        }
#endif
        return html;
    }
};

#endif // OSX_HTML_SIZING_PATCH_H
