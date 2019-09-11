/*
 *	IngeScape Common
 *
 *  Copyright © 2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Alexandre Lemort   <lemort@ingenuity.io>
 *
 */

#ifndef LINUXUTILS_H
#define LINUXUTILS_H

#include <QtGlobal>
#include <QObject>

#include "osutils.h"


#ifdef Q_OS_LINUX
//
// Linux definition
//


/**
 * @brief The LinuxUtils class defines defines a set of utility functions to perform Linux specific operations
 */
class LinuxUtils : public OSUtils
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     */
    explicit LinuxUtils(QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~LinuxUtils() Q_DECL_OVERRIDE;


    /**
     * @brief Get our singleton instance
     * @return
     */
    static LinuxUtils* instance();


    /**
     * @brief Remove all menu items generated by Qt to handle conventions
     */
    void removeOSGeneratedMenuItems() Q_DECL_OVERRIDE;


protected:
    /**
     * @brief Enable energy efficiency features
     */
    void _enableEnergyEfficiencyFeatures() Q_DECL_OVERRIDE;


    /**
     * @brief Disable energy efficiency features
     */
    void _disableEnergyEfficiencyFeatures() Q_DECL_OVERRIDE;

};


#else
//
// Non-Linux definition
//
class LinuxUtils : public OSUtils
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     */
    explicit LinuxUtils(QObject *parent = nullptr);

};


#endif

#endif // LINUXUTILS_H
