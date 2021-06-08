//
//  FolderView.hpp
//  Assignment2
//
//  Created by rick gessner on 2/15/21.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderView_h
#define FolderView_h

#include "View.hpp"

namespace ECE141 {

    class FolderView : public View {
    public:
        FolderView(const char* aPath, const char* anExtension = "db")
            : path(aPath), extension(anExtension) {}

        bool show(std::ostream& anOutput) override {
            return true;
        }

        const char* path;
        const char* extension;

    };

}

#endif /* FolderView_h */