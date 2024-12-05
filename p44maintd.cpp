//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  Copyright (c) 2024 plan44.ch / Lukas Zeller, Zurich, Switzerland
//
//  Author: Lukas Zeller <luz@plan44.ch>
//
//  This file is part of p44utils.
//
//  p44utils is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  p44utils is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with p44utils. If not, see <http://www.gnu.org/licenses/>.
//

#define FLASH_PATH "/flash/"
#define DEFAULT_DEFS_PATH "/etc/"
#define COMPUTING_MODULE_FILE "/tmp/p44-computing-module"

#define DEFAULT_LOGLEVEL LOG_EMERG // no logging by default

#include "application.hpp"

#include "jsonobject.hpp"
#include "macaddress.hpp"
#include "digitalio.hpp"
#include "utils.hpp"
#include "extutils.hpp"
#include "crc32.hpp"
#include "fnv.hpp"

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#if !BUILDENV_XCODE
  // Linux only
  #include <sys/ioctl.h>
  #include <sys/reboot.h>
  #include <sys/sysinfo.h>
#endif


using namespace p44;


#if BUILDENV_OPENWRT || BUILDENV_XCODE || BUILDENV_GENERIC
typedef struct {
  const char *tzname;
  const char *tzspec;
} TZInfo;

static const TZInfo timezones[] = {
  { "Africa/Abidjan", "GMT0" },
  { "Africa/Accra", "GMT0" },
  { "Africa/Addis Ababa", "EAT-3" },
  { "Africa/Algiers", "CET-1" },
  { "Africa/Asmara", "EAT-3" },
  { "Africa/Bamako", "GMT0" },
  { "Africa/Bangui", "WAT-1" },
  { "Africa/Banjul", "GMT0" },
  { "Africa/Bissau", "GMT0" },
  { "Africa/Blantyre", "CAT-2" },
  { "Africa/Brazzaville", "WAT-1" },
  { "Africa/Bujumbura", "CAT-2" },
  { "Africa/Cairo", "EET-2" },
  { "Africa/Casablanca", "<+01>-1" },
  { "Africa/Ceuta", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Africa/Conakry", "GMT0" },
  { "Africa/Dakar", "GMT0" },
  { "Africa/Dar es Salaam", "EAT-3" },
  { "Africa/Djibouti", "EAT-3" },
  { "Africa/Douala", "WAT-1" },
  { "Africa/El Aaiun", "<+01>-1" },
  { "Africa/Freetown", "GMT0" },
  { "Africa/Gaborone", "CAT-2" },
  { "Africa/Harare", "CAT-2" },
  { "Africa/Johannesburg", "SAST-2" },
  { "Africa/Juba", "EAT-3" },
  { "Africa/Kampala", "EAT-3" },
  { "Africa/Khartoum", "CAT-2" },
  { "Africa/Kigali", "CAT-2" },
  { "Africa/Kinshasa", "WAT-1" },
  { "Africa/Lagos", "WAT-1" },
  { "Africa/Libreville", "WAT-1" },
  { "Africa/Lome", "GMT0" },
  { "Africa/Luanda", "WAT-1" },
  { "Africa/Lubumbashi", "CAT-2" },
  { "Africa/Lusaka", "CAT-2" },
  { "Africa/Malabo", "WAT-1" },
  { "Africa/Maputo", "CAT-2" },
  { "Africa/Maseru", "SAST-2" },
  { "Africa/Mbabane", "SAST-2" },
  { "Africa/Mogadishu", "EAT-3" },
  { "Africa/Monrovia", "GMT0" },
  { "Africa/Nairobi", "EAT-3" },
  { "Africa/Ndjamena", "WAT-1" },
  { "Africa/Niamey", "WAT-1" },
  { "Africa/Nouakchott", "GMT0" },
  { "Africa/Ouagadougou", "GMT0" },
  { "Africa/Porto-Novo", "WAT-1" },
  { "Africa/Sao Tome", "GMT0" },
  { "Africa/Tripoli", "EET-2" },
  { "Africa/Tunis", "CET-1" },
  { "Africa/Windhoek", "CAT-2" },
  { "America/Adak", "HST10HDT,M3.2.0,M11.1.0" },
  { "America/Anchorage", "AKST9AKDT,M3.2.0,M11.1.0" },
  { "America/Anguilla", "AST4" },
  { "America/Antigua", "AST4" },
  { "America/Araguaina", "<-03>3" },
  { "America/Argentina/Buenos Aires", "<-03>3" },
  { "America/Argentina/Catamarca", "<-03>3" },
  { "America/Argentina/Cordoba", "<-03>3" },
  { "America/Argentina/Jujuy", "<-03>3" },
  { "America/Argentina/La Rioja", "<-03>3" },
  { "America/Argentina/Mendoza", "<-03>3" },
  { "America/Argentina/Rio Gallegos", "<-03>3" },
  { "America/Argentina/Salta", "<-03>3" },
  { "America/Argentina/San Juan", "<-03>3" },
  { "America/Argentina/San Luis", "<-03>3" },
  { "America/Argentina/Tucuman", "<-03>3" },
  { "America/Argentina/Ushuaia", "<-03>3" },
  { "America/Aruba", "AST4" },
  { "America/Asuncion", "<-04>4<-03>,M10.1.0/0,M3.4.0/0" },
  { "America/Atikokan", "EST5" },
  { "America/Bahia", "<-03>3" },
  { "America/Bahia Banderas", "CST6CDT,M4.1.0,M10.5.0" },
  { "America/Barbados", "AST4" },
  { "America/Belem", "<-03>3" },
  { "America/Belize", "CST6" },
  { "America/Blanc-Sablon", "AST4" },
  { "America/Boa Vista", "<-04>4" },
  { "America/Bogota", "<-05>5" },
  { "America/Boise", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Cambridge Bay", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Campo Grande", "<-04>4" },
  { "America/Cancun", "EST5" },
  { "America/Caracas", "<-04>4" },
  { "America/Cayenne", "<-03>3" },
  { "America/Cayman", "EST5" },
  { "America/Chicago", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Chihuahua", "MST7MDT,M4.1.0,M10.5.0" },
  { "America/Costa Rica", "CST6" },
  { "America/Creston", "MST7" },
  { "America/Cuiaba", "<-04>4" },
  { "America/Curacao", "AST4" },
  { "America/Danmarkshavn", "GMT0" },
  { "America/Dawson", "PST8PDT,M3.2.0,M11.1.0" },
  { "America/Dawson Creek", "MST7" },
  { "America/Denver", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Detroit", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Dominica", "AST4" },
  { "America/Edmonton", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Eirunepe", "<-05>5" },
  { "America/El Salvador", "CST6" },
  { "America/Fort Nelson", "MST7" },
  { "America/Fortaleza", "<-03>3" },
  { "America/Glace Bay", "AST4ADT,M3.2.0,M11.1.0" },
  { "America/Godthab", "<-03>3<-02>,M3.5.0/-2,M10.5.0/-1" },
  { "America/Goose Bay", "AST4ADT,M3.2.0,M11.1.0" },
  { "America/Grand Turk", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Grenada", "AST4" },
  { "America/Guadeloupe", "AST4" },
  { "America/Guatemala", "CST6" },
  { "America/Guayaquil", "<-05>5" },
  { "America/Guyana", "<-04>4" },
  { "America/Halifax", "AST4ADT,M3.2.0,M11.1.0" },
  { "America/Havana", "CST5CDT,M3.2.0/0,M11.1.0/1" },
  { "America/Hermosillo", "MST7" },
  { "America/Indiana/Indianapolis", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Indiana/Knox", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Indiana/Marengo", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Indiana/Petersburg", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Indiana/Tell City", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Indiana/Vevay", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Indiana/Vincennes", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Indiana/Winamac", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Inuvik", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Iqaluit", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Jamaica", "EST5" },
  { "America/Juneau", "AKST9AKDT,M3.2.0,M11.1.0" },
  { "America/Kentucky/Louisville", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Kentucky/Monticello", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Kralendijk", "AST4" },
  { "America/La Paz", "<-04>4" },
  { "America/Lima", "<-05>5" },
  { "America/Los Angeles", "PST8PDT,M3.2.0,M11.1.0" },
  { "America/Lower Princes", "AST4" },
  { "America/Maceio", "<-03>3" },
  { "America/Managua", "CST6" },
  { "America/Manaus", "<-04>4" },
  { "America/Marigot", "AST4" },
  { "America/Martinique", "AST4" },
  { "America/Matamoros", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Mazatlan", "MST7MDT,M4.1.0,M10.5.0" },
  { "America/Menominee", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Merida", "CST6CDT,M4.1.0,M10.5.0" },
  { "America/Metlakatla", "AKST9AKDT,M3.2.0,M11.1.0" },
  { "America/Mexico City", "CST6CDT,M4.1.0,M10.5.0" },
  { "America/Miquelon", "<-03>3<-02>,M3.2.0,M11.1.0" },
  { "America/Moncton", "AST4ADT,M3.2.0,M11.1.0" },
  { "America/Monterrey", "CST6CDT,M4.1.0,M10.5.0" },
  { "America/Montevideo", "<-03>3" },
  { "America/Montserrat", "AST4" },
  { "America/Nassau", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/New York", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Nipigon", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Nome", "AKST9AKDT,M3.2.0,M11.1.0" },
  { "America/Noronha", "<-02>2" },
  { "America/North Dakota/Beulah", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/North Dakota/Center", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/North Dakota/New Salem", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Ojinaga", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Panama", "EST5" },
  { "America/Pangnirtung", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Paramaribo", "<-03>3" },
  { "America/Phoenix", "MST7" },
  { "America/Port of Spain", "AST4" },
  { "America/Port-au-Prince", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Porto Velho", "<-04>4" },
  { "America/Puerto Rico", "AST4" },
  { "America/Punta Arenas", "<-03>3" },
  { "America/Rainy River", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Rankin Inlet", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Recife", "<-03>3" },
  { "America/Regina", "CST6" },
  { "America/Resolute", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Rio Branco", "<-05>5" },
  { "America/Santarem", "<-03>3" },
  { "America/Santiago", "<-04>4<-03>,M9.1.6/24,M4.1.6/24" },
  { "America/Santo Domingo", "AST4" },
  { "America/Sao Paulo", "<-03>3" },
  { "America/Scoresbysund", "<-01>1<+00>,M3.5.0/0,M10.5.0/1" },
  { "America/Sitka", "AKST9AKDT,M3.2.0,M11.1.0" },
  { "America/St Barthelemy", "AST4" },
  { "America/St Johns", "NST3:30NDT,M3.2.0,M11.1.0" },
  { "America/St Kitts", "AST4" },
  { "America/St Lucia", "AST4" },
  { "America/St Thomas", "AST4" },
  { "America/St Vincent", "AST4" },
  { "America/Swift Current", "CST6" },
  { "America/Tegucigalpa", "CST6" },
  { "America/Thule", "AST4ADT,M3.2.0,M11.1.0" },
  { "America/Thunder Bay", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Tijuana", "PST8PDT,M3.2.0,M11.1.0" },
  { "America/Toronto", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Tortola", "AST4" },
  { "America/Vancouver", "PST8PDT,M3.2.0,M11.1.0" },
  { "America/Whitehorse", "PST8PDT,M3.2.0,M11.1.0" },
  { "America/Winnipeg", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Yakutat", "AKST9AKDT,M3.2.0,M11.1.0" },
  { "America/Yellowknife", "MST7MDT,M3.2.0,M11.1.0" },
  { "Antarctica/Casey", "<+08>-8" },
  { "Antarctica/Davis", "<+07>-7" },
  { "Antarctica/DumontDUrville", "<+10>-10" },
  { "Antarctica/Macquarie", "<+11>-11" },
  { "Antarctica/Mawson", "<+05>-5" },
  { "Antarctica/McMurdo", "NZST-12NZDT,M9.5.0,M4.1.0/3" },
  { "Antarctica/Palmer", "<-03>3" },
  { "Antarctica/Rothera", "<-03>3" },
  { "Antarctica/Syowa", "<+03>-3" },
  { "Antarctica/Troll", "<+00>0<+02>-2,M3.5.0/1,M10.5.0/3" },
  { "Antarctica/Vostok", "<+06>-6" },
  { "Arctic/Longyearbyen", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Asia/Aden", "<+03>-3" },
  { "Asia/Almaty", "<+06>-6" },
  { "Asia/Amman", "EET-2EEST,M3.5.4/24,M10.5.5/1" },
  { "Asia/Anadyr", "<+12>-12" },
  { "Asia/Aqtau", "<+05>-5" },
  { "Asia/Aqtobe", "<+05>-5" },
  { "Asia/Ashgabat", "<+05>-5" },
  { "Asia/Atyrau", "<+05>-5" },
  { "Asia/Baghdad", "<+03>-3" },
  { "Asia/Bahrain", "<+03>-3" },
  { "Asia/Baku", "<+04>-4" },
  { "Asia/Bangkok", "<+07>-7" },
  { "Asia/Barnaul", "<+07>-7" },
  { "Asia/Beirut", "EET-2EEST,M3.5.0/0,M10.5.0/0" },
  { "Asia/Bishkek", "<+06>-6" },
  { "Asia/Brunei", "<+08>-8" },
  { "Asia/Chita", "<+09>-9" },
  { "Asia/Choibalsan", "<+08>-8" },
  { "Asia/Colombo", "<+0530>-5:30" },
  { "Asia/Damascus", "EET-2EEST,M3.5.5/0,M10.5.5/0" },
  { "Asia/Dhaka", "<+06>-6" },
  { "Asia/Dili", "<+09>-9" },
  { "Asia/Dubai", "<+04>-4" },
  { "Asia/Dushanbe", "<+05>-5" },
  { "Asia/Famagusta", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Asia/Gaza", "EET-2EEST,M3.5.5/0,M10.5.6/1" },
  { "Asia/Hebron", "EET-2EEST,M3.5.5/0,M10.5.6/1" },
  { "Asia/Ho Chi Minh", "<+07>-7" },
  { "Asia/Hong Kong", "HKT-8" },
  { "Asia/Hovd", "<+07>-7" },
  { "Asia/Irkutsk", "<+08>-8" },
  { "Asia/Jakarta", "WIB-7" },
  { "Asia/Jayapura", "WIT-9" },
  { "Asia/Jerusalem", "IST-2IDT,M3.4.4/26,M10.5.0" },
  { "Asia/Kabul", "<+0430>-4:30" },
  { "Asia/Kamchatka", "<+12>-12" },
  { "Asia/Karachi", "PKT-5" },
  { "Asia/Kathmandu", "<+0545>-5:45" },
  { "Asia/Khandyga", "<+09>-9" },
  { "Asia/Kolkata", "IST-5:30" },
  { "Asia/Krasnoyarsk", "<+07>-7" },
  { "Asia/Kuala Lumpur", "<+08>-8" },
  { "Asia/Kuching", "<+08>-8" },
  { "Asia/Kuwait", "<+03>-3" },
  { "Asia/Macau", "CST-8" },
  { "Asia/Magadan", "<+11>-11" },
  { "Asia/Makassar", "WITA-8" },
  { "Asia/Manila", "PST-8" },
  { "Asia/Muscat", "<+04>-4" },
  { "Asia/Nicosia", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Asia/Novokuznetsk", "<+07>-7" },
  { "Asia/Novosibirsk", "<+07>-7" },
  { "Asia/Omsk", "<+06>-6" },
  { "Asia/Oral", "<+05>-5" },
  { "Asia/Phnom Penh", "<+07>-7" },
  { "Asia/Pontianak", "WIB-7" },
  { "Asia/Pyongyang", "KST-9" },
  { "Asia/Qatar", "<+03>-3" },
  { "Asia/Qostanay", "<+06>-6" },
  { "Asia/Qyzylorda", "<+05>-5" },
  { "Asia/Riyadh", "<+03>-3" },
  { "Asia/Sakhalin", "<+11>-11" },
  { "Asia/Samarkand", "<+05>-5" },
  { "Asia/Seoul", "KST-9" },
  { "Asia/Shanghai", "CST-8" },
  { "Asia/Singapore", "<+08>-8" },
  { "Asia/Srednekolymsk", "<+11>-11" },
  { "Asia/Taipei", "CST-8" },
  { "Asia/Tashkent", "<+05>-5" },
  { "Asia/Tbilisi", "<+04>-4" },
  { "Asia/Tehran", "<+0330>-3:30<+0430>,J79/24,J263/24" },
  { "Asia/Thimphu", "<+06>-6" },
  { "Asia/Tokyo", "JST-9" },
  { "Asia/Tomsk", "<+07>-7" },
  { "Asia/Ulaanbaatar", "<+08>-8" },
  { "Asia/Urumqi", "<+06>-6" },
  { "Asia/Ust-Nera", "<+10>-10" },
  { "Asia/Vientiane", "<+07>-7" },
  { "Asia/Vladivostok", "<+10>-10" },
  { "Asia/Yakutsk", "<+09>-9" },
  { "Asia/Yangon", "<+0630>-6:30" },
  { "Asia/Yekaterinburg", "<+05>-5" },
  { "Asia/Yerevan", "<+04>-4" },
  { "Atlantic/Azores", "<-01>1<+00>,M3.5.0/0,M10.5.0/1" },
  { "Atlantic/Bermuda", "AST4ADT,M3.2.0,M11.1.0" },
  { "Atlantic/Canary", "WET0WEST,M3.5.0/1,M10.5.0" },
  { "Atlantic/Cape Verde", "<-01>1" },
  { "Atlantic/Faroe", "WET0WEST,M3.5.0/1,M10.5.0" },
  { "Atlantic/Madeira", "WET0WEST,M3.5.0/1,M10.5.0" },
  { "Atlantic/Reykjavik", "GMT0" },
  { "Atlantic/South Georgia", "<-02>2" },
  { "Atlantic/St Helena", "GMT0" },
  { "Atlantic/Stanley", "<-03>3" },
  { "Australia/Adelaide", "ACST-9:30ACDT,M10.1.0,M4.1.0/3" },
  { "Australia/Brisbane", "AEST-10" },
  { "Australia/Broken Hill", "ACST-9:30ACDT,M10.1.0,M4.1.0/3" },
  { "Australia/Currie", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
  { "Australia/Darwin", "ACST-9:30" },
  { "Australia/Eucla", "<+0845>-8:45" },
  { "Australia/Hobart", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
  { "Australia/Lindeman", "AEST-10" },
  { "Australia/Lord Howe", "<+1030>-10:30<+11>-11,M10.1.0,M4.1.0" },
  { "Australia/Melbourne", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
  { "Australia/Perth", "AWST-8" },
  { "Australia/Sydney", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
  { "Etc/GMT", "GMT0" },
  { "Etc/GMT+1", "<-01>1" },
  { "Etc/GMT+10", "<-10>10" },
  { "Etc/GMT+11", "<-11>11" },
  { "Etc/GMT+12", "<-12>12" },
  { "Etc/GMT+2", "<-02>2" },
  { "Etc/GMT+3", "<-03>3" },
  { "Etc/GMT+4", "<-04>4" },
  { "Etc/GMT+5", "<-05>5" },
  { "Etc/GMT+6", "<-06>6" },
  { "Etc/GMT+7", "<-07>7" },
  { "Etc/GMT+8", "<-08>8" },
  { "Etc/GMT+9", "<-09>9" },
  { "Etc/GMT-1", "<+01>-1" },
  { "Etc/GMT-10", "<+10>-10" },
  { "Etc/GMT-11", "<+11>-11" },
  { "Etc/GMT-12", "<+12>-12" },
  { "Etc/GMT-13", "<+13>-13" },
  { "Etc/GMT-14", "<+14>-14" },
  { "Etc/GMT-2", "<+02>-2" },
  { "Etc/GMT-3", "<+03>-3" },
  { "Etc/GMT-4", "<+04>-4" },
  { "Etc/GMT-5", "<+05>-5" },
  { "Etc/GMT-6", "<+06>-6" },
  { "Etc/GMT-7", "<+07>-7" },
  { "Etc/GMT-8", "<+08>-8" },
  { "Etc/GMT-9", "<+09>-9" },
  { "Europe/Amsterdam", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Andorra", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Astrakhan", "<+04>-4" },
  { "Europe/Athens", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Belgrade", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Berlin", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Bratislava", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Brussels", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Bucharest", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Budapest", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Busingen", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Chisinau", "EET-2EEST,M3.5.0,M10.5.0/3" },
  { "Europe/Copenhagen", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Dublin", "IST-1GMT0,M10.5.0,M3.5.0/1" },
  { "Europe/Gibraltar", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Guernsey", "GMT0BST,M3.5.0/1,M10.5.0" },
  { "Europe/Helsinki", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Isle of Man", "GMT0BST,M3.5.0/1,M10.5.0" },
  { "Europe/Istanbul", "<+03>-3" },
  { "Europe/Jersey", "GMT0BST,M3.5.0/1,M10.5.0" },
  { "Europe/Kaliningrad", "EET-2" },
  { "Europe/Kiev", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Kirov", "<+03>-3" },
  { "Europe/Lisbon", "WET0WEST,M3.5.0/1,M10.5.0" },
  { "Europe/Ljubljana", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/London", "GMT0BST,M3.5.0/1,M10.5.0" },
  { "Europe/Luxembourg", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Madrid", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Malta", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Mariehamn", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Minsk", "<+03>-3" },
  { "Europe/Monaco", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Moscow", "MSK-3" },
  { "Europe/Oslo", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Paris", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Podgorica", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Prague", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Riga", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Rome", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Samara", "<+04>-4" },
  { "Europe/San Marino", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Sarajevo", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Saratov", "<+04>-4" },
  { "Europe/Simferopol", "MSK-3" },
  { "Europe/Skopje", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Sofia", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Stockholm", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Tallinn", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Tirane", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Ulyanovsk", "<+04>-4" },
  { "Europe/Uzhgorod", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Vaduz", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Vatican", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Vienna", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Vilnius", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Volgograd", "<+04>-4" },
  { "Europe/Warsaw", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Zagreb", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Zaporozhye", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Zurich", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Indian/Antananarivo", "EAT-3" },
  { "Indian/Chagos", "<+06>-6" },
  { "Indian/Christmas", "<+07>-7" },
  { "Indian/Cocos", "<+0630>-6:30" },
  { "Indian/Comoro", "EAT-3" },
  { "Indian/Kerguelen", "<+05>-5" },
  { "Indian/Mahe", "<+04>-4" },
  { "Indian/Maldives", "<+05>-5" },
  { "Indian/Mauritius", "<+04>-4" },
  { "Indian/Mayotte", "EAT-3" },
  { "Indian/Reunion", "<+04>-4" },
  { "Pacific/Apia", "<+13>-13<+14>,M9.5.0/3,M4.1.0/4" },
  { "Pacific/Auckland", "NZST-12NZDT,M9.5.0,M4.1.0/3" },
  { "Pacific/Bougainville", "<+11>-11" },
  { "Pacific/Chatham", "<+1245>-12:45<+1345>,M9.5.0/2:45,M4.1.0/3:45" },
  { "Pacific/Chuuk", "<+10>-10" },
  { "Pacific/Easter", "<-06>6<-05>,M9.1.6/22,M4.1.6/22" },
  { "Pacific/Efate", "<+11>-11" },
  { "Pacific/Enderbury", "<+13>-13" },
  { "Pacific/Fakaofo", "<+13>-13" },
  { "Pacific/Fiji", "<+12>-12<+13>,M11.2.0,M1.2.3/99" },
  { "Pacific/Funafuti", "<+12>-12" },
  { "Pacific/Galapagos", "<-06>6" },
  { "Pacific/Gambier", "<-09>9" },
  { "Pacific/Guadalcanal", "<+11>-11" },
  { "Pacific/Guam", "ChST-10" },
  { "Pacific/Honolulu", "HST10" },
  { "Pacific/Kiritimati", "<+14>-14" },
  { "Pacific/Kosrae", "<+11>-11" },
  { "Pacific/Kwajalein", "<+12>-12" },
  { "Pacific/Majuro", "<+12>-12" },
  { "Pacific/Marquesas", "<-0930>9:30" },
  { "Pacific/Midway", "SST11" },
  { "Pacific/Nauru", "<+12>-12" },
  { "Pacific/Niue", "<-11>11" },
  { "Pacific/Norfolk", "<+11>-11<+12>,M10.1.0,M4.1.0/3" },
  { "Pacific/Noumea", "<+11>-11" },
  { "Pacific/Pago Pago", "SST11" },
  { "Pacific/Palau", "<+09>-9" },
  { "Pacific/Pitcairn", "<-08>8" },
  { "Pacific/Pohnpei", "<+11>-11" },
  { "Pacific/Port Moresby", "<+10>-10" },
  { "Pacific/Rarotonga", "<-10>10" },
  { "Pacific/Saipan", "ChST-10" },
  { "Pacific/Tahiti", "<-10>10" },
  { "Pacific/Tarawa", "<+12>-12" },
  { "Pacific/Tongatapu", "<+13>-13" },
  { "Pacific/Wake", "<+12>-12" },
  { "Pacific/Wallis", "<+12>-12" },
  { NULL, NULL } // terminator
};

#endif // BUILDENV_OPENWRT || BUILDENV_XCODE


extern char **environ;

bool checkParam(JsonObjectPtr aParams, const char *aParamName, JsonObjectPtr &aParam)
{
  bool exists = false;
  aParam.reset();
  if (aParams)
    exists = aParams->get(aParamName, aParam, false);
  return exists;
}


bool checkStringParam(JsonObjectPtr aParams, const char *aParamName, string &aParamValue)
{
  JsonObjectPtr o;
  bool found = checkParam(aParams, aParamName, o);
  if (found) {
    aParamValue = o->stringValue();
  }
  return found;
}


static const CmdLineOptionDescriptor options[] = {
  #ifdef ADDITIONAL_OPTIONS
  ADDITIONAL_OPTIONS
  #endif
  { 0  , "json",            true,  "jsonquery;process JSON config/maintainance command" },
  { 0  , "factoryreset",    true,  "mode;factory reset, mode: 1=reset dS settings, 2=reset network settings, 3=reset both" },
  { 0  , "defs",            false, "output all platform, product and unit defs as shell var assignments" },
  { 0  , "defsdir",         true,  "dir;directory where to read .defs files and pubkey from, defaults to " DEFAULT_DEFS_PATH },
  { 'i', "deviceinfo",      false, "human readable device info" },
  { 'l', "loglevel",        true,  "level;set max level of log message detail to show on stderr" },
  { 0  , "deltatstamps",    false, "show timestamp delta between log lines" },
  { 'V', "version",         false, "show version" },
  { 'h', "help",            false, "show this text" },
  { 0, NULL } // list terminator
};


class P44maintd : public CmdLineApp
{
  typedef CmdLineApp inherited;

protected:

  // Indicator
  IndicatorOutputPtr redLED;
  IndicatorOutputPtr greenLED;

  // system config
  string defspath;
  typedef map<string, string> DefsMap;
  DefsMap defs;

public:

  P44maintd()
  {
    defspath = DEFAULT_DEFS_PATH;
    // set dummy LEDs
    redLED = IndicatorOutputPtr (new IndicatorOutput("missing", false));
    greenLED = IndicatorOutputPtr (new IndicatorOutput("missing", false));
  }


  virtual int main(int argc, char **argv)
  {
    const char *usageText = "Usage: %1$s [options]\n";

    // parse the command line, exits when syntax errors occur
    setCommandDescriptors(usageText, options);
    if (parseCommandLine(argc, argv)) {
      if (numOptions()<1) {
        // show usage
        showUsage();
        terminateApp(EXIT_SUCCESS);
      }
      else {
        // different defs dir?
        getStringOption("defsdir", defspath);
        if (defspath.size()>0 && defspath[defspath.size()-1]!='/')
          defspath += '/';

        // log level?
        int loglevel = DEFAULT_LOGLEVEL;
        getIntOption("loglevel", loglevel);
        SETLOGLEVEL(loglevel);
        SETERRLEVEL(loglevel, false); // all diagnostics go to stderr
        SETDELTATIME(getOption("deltatstamps"));
      }
    }

    // app now ready to run
    return run();
  }


  void enableLEDs()
  {
    // use platform defs to determine which are the LEDs
    string io;
    if (getDef("PLATFORM_RED_LED", io)) {
      redLED = IndicatorOutputPtr(new IndicatorOutput(io.c_str(), false));
    }
    if (getDef("PLATFORM_GREEN_LED", io)) {
      greenLED = IndicatorOutputPtr(new IndicatorOutput(io.c_str(), false));
    }
  }


  uint64_t serial()
  {
    uint64_t mac = macAddress();
    // lower 24bits are 1:1 from MAC
    uint64_t serial = mac & 0xFFFFFF;
    // check for plan44-used MAC OUIs
    if ((mac>>24)==0x00409D) {
      // Digiboard Inc. aka digi.com
      serial = serial | (1<<24); // 1 = plan44 serial encoding of Digiboard Inc. OUI
    }
    else if ((mac>>24)==0xB827EB) {
      // Raspberry Pi foundation
      serial = serial | (2<<24); // 2 = plan44 serial encoding of Raspberry Pi Foundation
    }
    else if ((mac>>24)==0x40A36B) {
      // Onion Corporation (C00000-CFFFFF)
      serial = serial | (3<<24); // 3 = plan44 serial encoding of Onion Corporation
    }
    else {
      // unknown OUI, UA must do
      serial = serial | (42<<24);
    }
    return serial;
  }


  uint64_t macFromSerial(uint64_t aSerial)
  {
    // lower 24bits are 1:1 from serial
    uint64_t mac = aSerial & 0xFFFFFF;
    switch (aSerial>>24) {
      case 1: mac |= (0x00409Dul<<24); break; // Digiboard Inc
      case 2: mac |= (0xB827EBul<<24); break; // Raspberry Pi Foundation
      case 3: mac |= (0x40A36Bul<<24); break; // Onion Corporation
    }
    return mac;
  }


  bool readDefsFrom(string aFileName, DefsMap &aDefs)
  {
    string line;
    bool readAnything = false;
    FILE *file = fopen(aFileName.c_str(), "r");
    if (file) {
      // file opened
      string l, key, value;
      while (string_fgetline(file, line)) {
        l = trimWhiteSpace(line, true, false);
        if (l.size()>0 && l[0]!='#') {
          // not comment
          if (keyAndValue(line, key, value, '=')) {
            // key/value
            if (value.size()>0 && (value[0]=='"' || value[0]=='\'')) {
              // consider quoted, means that \ is escape and string ends when quote appears again
              string dequoted;
              char quote = value[0];
              size_t i=1;
              bool escaped = false;
              while (i<value.size()) {
                if (!escaped) {
                  if (value[i]==quote) break;
                  if (value[i]=='\\') {
                    escaped = true;
                    i++;
                    continue;
                  }
                }
                dequoted += value[i];
                escaped = false;
                i++;
              }
              aDefs[key] = dequoted;
            }
            else {
              // use as-is
              aDefs[key] = value;
            }
            readAnything = true;
          }
        }
      }
      fclose(file);
    }
    return readAnything;
  }


  bool readDefFromFirstLine(const string aFileName, const char *key)
  {
    string value;
    if (string_fgetfirstline(aFileName, value)) {
      defs[key] = value;
      return true;
    }
    return false;
  }


  bool getDef(const string aKey, string &aDef, const char *aDefault=NULL, DefsMap *aDefsP = NULL)
  {
    DefsMap &myDefs = aDefsP ? *aDefsP : defs;
    DefsMap::iterator pos;
    pos = myDefs.find(aKey);
    if (pos!=myDefs.end()) {
      aDef = pos->second;
      return true;
    }
    if (aDefault) {
      aDef = aDefault;
    }
    return false;
  }


  string getDef(const string aKey, DefsMap *aDefsP = NULL)
  {
    string res;
    getDef(aKey, res, NULL, aDefsP);
    return res;
  }


  // set value if not already defined
  bool setDefDefault(const string aKey, const string aValue)
  {
    if (defs.find(aKey)!=defs.end()) return false;
    defs[aKey] = aValue;
    return true;
  }



  // "ok", "T", "t", "Y", "y", "1" are all considered true, everything else means false
  bool isDefTrue(const string aKey)
  {
    string def;
    if (getDef(aKey, def)) {
      if (def.size()>0 && (def=="1" || def=="ok" || tolower(def[0])=='t' || tolower(def[0])=='y')) {
        return true;
      }
    }
    return false;
  }





  // MARK: ===== identification of the device

  virtual bool setDefDefaults()
  {
    // in all cases: current time
    defs["STATUS_TIME"] = string_ftime("%Y-%m-%d %H:%M:%S");
    #if BUILDENV_XCODE
    // pseudo-platform has fixed defs, without loading anything
    // - platform
    defs["PLATFORM_IDENTIFIER"] = "xcode_dummy";
    defs["PLATFORM_NAME"] = "MacOSX";
    // - product
    defs["PRODUCT_IDENTIFIER"] = "p44-xx-mac-xcode";
    defs["PRODUCT_MODEL"] = "P44-XX-MAC";
    defs["PRODUCT_VARIANT"] = "Apple";
    defs["PRODUCT_HOSTPREFIX"] = "p44_xx_mac";
    // - firmware
    defs["FIRMWARE_VERSION"] = "0.0.0.42";
    defs["FIRMWARE_FEED"] = "opensource";
    // - status
    defs["STATUS_USER_LEVEL"] = "0";
    // skip dynamic platform stuff for XCode builds
    return false;
    #elif BUILDENV_GENERIC
    // pseudo-platform has fixed defs, without loading anything
    // - platform
    defs["PLATFORM_IDENTIFIER"] = "generic_dummy";
    defs["PLATFORM_NAME"] = "Linux";
    defs["PLATFORM_SERIALDEV"] = "/dev/null";
    defs["PLATFORM_DALIDEV"] = "/dev/null";
    // - product
    defs["PRODUCT_IDENTIFIER"] = "p44-xx-linux-generic";
    defs["PRODUCT_MODEL"] = "P44-XX-LINUX";
    defs["PRODUCT_VARIANT"] = "Debian";
    defs["PRODUCT_HOSTPREFIX"] = "p44_xx_linux";
    defs["PRODUCT_HAS_TINKER"] = "1";
    defs["PRODUCT_RESTART_TIME"] = "5";
    // - producer
    defs["PRODUCER"] = "plan44";
    // - firmware
    defs["FIRMWARE_VERSION"] = "0.0.0.42";
    defs["FIRMWARE_FEED"] = "devel";
    // - status
    defs["STATUS_USER_LEVEL"] = "0";
    // skip dynamic platform stuff for Generic Linux builds
    return false;
    #else
    // determine platform dynamically
    return true;
    #endif
  }


  // add dynamically obtainable platform identification info
  void identifyDynamically(SimpleCB aCallback)
  {
    string def;

    // build defs
    defs.clear();
    // set defaults
    if (!setDefDefaults()) {
      // defaults are already sufficient for platform
      processProductSpecifics(aCallback);
    }
    else {
      // read defs files to dettermine platform
      // - platform, possibly is a softlink
      readDefsFrom(defspath+"p44platform.defs", defs);
      // - this might be a generic head definition file in a FW that supports multiple platforms.
      //   Either it contains a PLATFORM_IDENTIFIER, or it might also contain a PLATFORM_IDENTIFIER_GETTER
      //   (which can also override a default PLATFORM_IDENTIFIER already present at this point)
      if (getDef("PLATFORM_IDENTIFIER_GETTER", def)) {
        MainLoop::currentMainLoop().fork_and_system(
          boost::bind(&P44maintd::platformidQueryDone, this, aCallback, _1, _2),
          def.c_str(),
          true, NULL, // collect stdout into string
          0 // mute stderr
        );
        return;
      }
      // if we get here, there is no PLATFORM_IDENTIFIER_GETTER that might provide/override PLATFORM_IDENTIFIER
      processPlatformSpecifics(aCallback);
    }
  }


  void platformidQueryDone(SimpleCB aCallback, ErrorPtr err, const string &aAnswer)
  {
    string v = trimWhiteSpace(aAnswer);
    if (v.size()>0) {
      defs["PLATFORM_IDENTIFIER"] = v;
    }
    processPlatformSpecifics(aCallback);
  }


  void processPlatformSpecifics(SimpleCB aCallback)
  {
    string def;

    // - additional platform definitions that may be included in the common firmware for multiple platforms
    if (getDef("PLATFORM_IDENTIFIER", def)) {
      readDefsFrom(defspath+"p44platform-" + def + ".defs", defs);
    }
    // - set/override runtime detected computing module (Note: usually available only after p44 init script has run)
    readDefFromFirstLine(COMPUTING_MODULE_FILE, "PLATFORM_COMPUTINGMODULE");
    // check for dynamic product ID getter
    //  such as: "/sbin/ubootenv --print 'p44productid' | sed -r -n -e '/^p44productid=/s/p44productid=//p'"
    if (getDef("PLATFORM_PRODUCT_IDENTIFIER_GETTER", def)) {
      MainLoop::currentMainLoop().fork_and_system(
        boost::bind(&P44maintd::productidQueryDone, this, aCallback, _1, _2),
        def.c_str(),
        true, NULL, // collect stdout into string
        0 // mute stderr
      );
      return;
    }
    // if we get here, product identifier is already there, so we can continue processing the product specifics
    processProductSpecifics(aCallback);
  }


  void productidQueryDone(SimpleCB aCallback, ErrorPtr err, const string &aAnswer)
  {
    string v = trimWhiteSpace(aAnswer);
    if (v.size()>0) {
      defs["PRODUCT_IDENTIFIER"] = v;
    }
    processProductSpecifics(aCallback);
  }


  void processProductSpecifics(SimpleCB aCallback)
  {
    string def;

    // - product, possibly is a softlink
    readDefsFrom(defspath+"p44product.defs", defs);
    // - if neither PLATFORM_PRODUCT_IDENTIFIER_GETTER nor p44product.defs did  deliver a product identifier, try to load default
    if (!getDef("PRODUCT_IDENTIFIER", def)) {
      if (getDef("PLATFORM_IDENTIFIER", def)) {
        // platform specific
        readDefsFrom(defspath+"p44product-default_" + def + ".defs", defs);
      }
    }
    if (!getDef("PRODUCT_IDENTIFIER", def)) {
      // still none - try generic defaults
      readDefsFrom(defspath+"p44product-default.defs", defs);
    }
    // - additional product definitions that may included in the common firmware for multiple products
    if (getDef("PRODUCT_IDENTIFIER", def)) {
      readDefsFrom(defspath+"p44product-" + def + ".defs", defs);
    }
    // check for dynamic producer
    //  such as: "fw_printenv p44producer | sed -r -n -e '/^p44producer=/s/.*=//p'"
    if (getDef("PRODUCER_GETTER", def)) {
      // obtain producer
      MainLoop::currentMainLoop().fork_and_system(
        boost::bind(&P44maintd::producerQueryDone, this, aCallback, _1, _2),
        def.c_str(),
        true, NULL, // collect stdout into string
        0 // mute stderr
      );
      return;
    }
    else {
      // assume static producer
      // - check separate file first
      readDefFromFirstLine(defspath+"p44producer", "PRODUCER");
      checkProducer(aCallback);
    }
  }


  void producerQueryDone(SimpleCB aCallback, ErrorPtr err, const string &aAnswer)
  {
    string v = trimWhiteSpace(aAnswer);
    if (v.size()>0) {
      defs["PRODUCER"] = v;
    }
    checkProducer(aCallback);
  }


  void checkProducer(SimpleCB aCallback)
  {
    string def;

    // - make sure we have at least a "unknown" producer
    setDefDefault("PRODUCER", "unknown");
    // - feed
    readDefFromFirstLine(defspath+"p44feed", "FIRMWARE_FEED");
    // - version
    readDefFromFirstLine(defspath+"p44version", "FIRMWARE_VERSION");
    // - user level
    if (!readDefFromFirstLine("/tmp/p44userlevel", "STATUS_USER_LEVEL")) {
      if (!readDefFromFirstLine(FLASH_PATH "p44userlevel", "STATUS_USER_LEVEL")) {
        if (getDef("PRODUCT_DEFAULT_USER_LEVEL", def)) {
          // use product specific default user level
          defs["STATUS_USER_LEVEL"] = def;
        }
        else {
          // production default is 0, testing/beta/development default is 1
          defs["STATUS_USER_LEVEL"] = getDef("FIRMWARE_FEED")=="prod" ? "0" : "1";
        }
      }
    }
    // check for dynamic variant getter
    //  such as: "/sbin/ubootenv --print 'p44variant' | sed -r -n -e '/^p44variant=/s/p44variant=//p'"
    //  or: "cat /boot/p44variant"
    if (getDef("PLATFORM_VARIANT_GETTER", def)) {
      MainLoop::currentMainLoop().fork_and_system(
        boost::bind(&P44maintd::variantQueryDone, this, aCallback, _1, _2),
        def.c_str(),
        true, NULL, // collect stdout into string
        0 // mute stderr
      );
      return;
    }
    // if we get here, variant info is already there, so we can continue processing it
    processVariantSpecifics(aCallback);
  }


  void variantQueryDone(SimpleCB aCallback, ErrorPtr err, const string &aAnswer)
  {
    string v = trimWhiteSpace(aAnswer);
    if (v.size()>0) {
      defs["PRODUCT_VARIANT"] = v;
    }
    else {
      // assume variant 0 if not set
      defs["PRODUCT_VARIANT"] = "0"; // e.g. DEH v3
    }
    processVariantSpecifics(aCallback);
  }


  virtual void setDerivedDefs()
  {
    // - copyright range
    struct timeval t;
    gettimeofday(&t, NULL);
    struct tm *tim = localtime(&t.tv_sec);
    setDefDefault("PRODUCT_COPYRIGHT_YEARS", string_format("2013-%04d", tim->tm_year+1900));
    // - copyright holder
    setDefDefault("PRODUCT_COPYRIGHT_HOLDER", "plan44.ch");
  }


  void processVariantSpecifics(SimpleCB aCallback)
  {
    string def;

    // try to load product variant specific settings
    if (getDef("PRODUCT_VARIANT", def)) {
      readDefsFrom(defspath+"p44variant-" + getDef("PRODUCT_IDENTIFIER") + "-" + def + ".defs", defs);
    }
    // overrides from individual configuration
    readDefsFrom(FLASH_PATH "/p44custom.defs", defs);
    // get unit variables
    // - serial
    defs["UNIT_SERIALNO"] = string_format("%lld", serial());
    // - MAC address
    uint64_t mac = macAddress();
    string macStr;
    defs["UNIT_MAC_DECIMAL"] = string_format("%lld", mac);
    for (int i=0; i<6; ++i) {
      if (i>0) macStr += ":";
      string_format_append(macStr, "%02X",(unsigned int)((mac>>((5-i)*8)) & 0xFF));
    }
    defs["UNIT_MACADDRESS"] = macStr;
    // - IPv4
    uint32_t ipv4 = ipv4Address();
    defs["STATUS_IPV4"] = string_format("%d.%d.%d.%d", (ipv4>>24) & 0xFF, (ipv4>>16) & 0xFF, (ipv4>>8) & 0xFF, ipv4 & 0xFF);
    // - host name
    getDef("PRODUCT_HOSTPREFIX", def, "unknown");
    defs["UNIT_HOSTNAME"] = string_format("%s_%lld",def.c_str(), serial());
    // Derived default values
    setDerivedDefs();
    // done
    aCallback();
  }


  int userlevel()
  {
    string def;
    int userlevel = 0;
    if (getDef("STATUS_USER_LEVEL", def)) {
      sscanf(def.c_str(), "%d", &userlevel);
    }
    return userlevel;
  }


  // MARK: ===== command line actions

  virtual void initialize()
  {
    // need full platform identification first
    identifyDynamically(boost::bind(&P44maintd::platformCommands, this));
  }


  // check commands that need full platform identification before being run
  virtual void platformCommands()
  {
    // check operation to perform
    const char *jsonCommand;
    int intOpt;
    if (getStringOption("json", jsonCommand)) {
      // process JSON command line call
      processJSON(jsonCommand);
    }
    else if (getOption("deviceinfo")) {
      // show device info
      showDeviceInfo();
    }
    else if (getOption("defs")) {
      // show device info
      showDefs();
    }
    else if (getIntOption("factoryreset", intOpt)) {
      // factory reset
      factoryReset(intOpt);
    }
    else {
      // no operation
      terminateApp(EXIT_FAILURE);
    }
  }


  // MARK: ===== reboot


  virtual void watchdog_arm(int aTimeoutSeconds)
  {
    // not implemented
  }


  void system_reboot(bool aHardReset, bool aPowerOff)
  {
    // signal "busy"
    enableLEDs();
    redLED->steadyOn();
    greenLED->steadyOn();
    // Now reboot
    if (aHardReset) {
      #if !BUILDENV_XCODE && !BUILDENV_GENERIC
      // use the watchdog as a fallback
      watchdog_arm(10); // ten seconds later, watchdog should hit anyway
      // but issue hard Linux system reset / power off right now
      reboot(aPowerOff ? RB_POWER_OFF : RB_AUTOBOOT);
      #endif
    }
    else {
      // reboot
      watchdog_arm(3*60); // as a fallback, trigger watchdog 3 minutes later
      string sdcmd = string_format("sv stop p44mbrd vdcd mg44; sync; %s", aPowerOff ? "poweroff" : "reboot");
      MainLoop::currentMainLoop().fork_and_system(
        NoOP,
        #if !BUILDENV_XCODE && !BUILDENV_GENERIC
        sdcmd.c_str()
        #else
        "echo dummy call simulating restart or shutdown"
        #endif
        ,true, NULL, // capture output to prevent output going to mg44
        0 // mute stderr
      );
    }
  }


  // MARK: ===== JSON interface for web


  JsonObjectPtr makeAnswer(JsonObjectPtr aResultObj)
  {
    // create answer
    JsonObjectPtr answer = JsonObject::newObj();
    if (!aResultObj) aResultObj = JsonObject::newNull();
    answer->add("result", aResultObj);
    return answer;
  }


  JsonObjectPtr makeErrorAnswer(ErrorPtr aError)
  {
    // create error answer
    JsonObjectPtr answer = JsonObject::newObj();
    JsonObjectPtr e = JsonObject::newObj();
    e->add("code", JsonObject::newInt32((int)aError->getErrorCode()));
    e->add("message", JsonObject::newString(aError->getErrorMessage()));
    answer->add("error", e);
    return answer;
  }


  JsonObjectPtr emptyAnswer()
  {
    // create null answer
    return makeAnswer(JsonObject::newNull());
  }


  JsonObjectPtr statusAnswer(ErrorPtr aError, JsonObjectPtr aResultObj = JsonObjectPtr())
  {
    if (Error::isOK(aError)) {
      return makeAnswer(aResultObj);
    }
    else {
      return makeErrorAnswer(aError);
    }
  }


  void answer(JsonObjectPtr aJSONAnswer)
  {
    if (aJSONAnswer) {
      LOG(LOG_DEBUG, "Replying with JSON answer: '%s'", aJSONAnswer->json_c_str());
      puts(aJSONAnswer->json_c_str());
    }
  }


  void answerAndTerminate(JsonObjectPtr aJSONAnswer)
  {
    answer(aJSONAnswer);
    terminateApp(EXIT_SUCCESS);
  }


  virtual ErrorPtr handleJSONCmd(string aCmd, JsonObjectPtr aParams, JsonObjectPtr aCmdObj, JsonObjectPtr& aAnswer)
  {
    ErrorPtr err;

    // decode command
    if (aCmd=="restart") {
      aAnswer = restart_from_ui(err, false);
    }
    else if (aCmd=="poweroff") {
      aAnswer = restart_from_ui(err, true);
    }
    else if (aCmd=="configbackup") {
      config_backup(err);
    }
    else if (aCmd=="configrestoreprep") {
      aAnswer = config_restore_prep(aCmdObj, err); // uploadedfile is not a uri_param nor post data, but one level up
    }
    else if (aCmd=="configrestoreapply") {
      aAnswer = config_restore_apply(aParams, err);
    }
    #if !BUILDENV_DIGIESP
    else if (aCmd=="tzconfig") {
      aAnswer = timezoneconfig(aParams, err);
    }
    else if (aCmd=="wificonfig") {
      aAnswer = wificonfig(aParams, err);
    }
    #endif // !BUILDENV_DIGIESP
    else if (aCmd=="ipconfig") {
      aAnswer = ipconfig(aParams, err);
    }
    else if (aCmd=="setpassword") {
      aAnswer = setpassword(aParams, err);
    }
    else if (aCmd=="factoryreset") {
      aAnswer = factory_reset_from_ui(aParams, err);
    }
    else if (aCmd=="devinfo") {
      aAnswer = devinfo(err);
    }
    else if (aCmd=="userlevel") {
      aAnswer = userlevelaccess(aParams, err);
    }
    else if (aCmd=="property") {
      aAnswer = property(aParams, err);
    }
    else if (aCmd=="alert") {
      aAnswer = alert_from_ui(aParams, err);
    }
    else {
      err = ErrorPtr(new Error(1,"Unknown 'cmd'"));
    }
    return err;
  }


  void processJSON(const char *aJSONCommand)
  {
    LOG(LOG_DEBUG, "Received command line JSON call: '%s'", aJSONCommand);
    JsonObjectPtr cmdObj = JsonObject::objFromText(aJSONCommand);
    ErrorPtr err;
    JsonObjectPtr answer;
    if (cmdObj) {
      // { "method":"GET", "uri":"aga", "uri_params": {"cmd": "ipconfig", "ipaddr": "1.2.3.4", "netmask": "255.255.255.0", "dhcp": 0, "gw":"1.2.3.1" } }
      // { "method":"POST", "uri":"aga", "data": {"cmd": "ipconfig", "ipaddr": "1.2.3.4", "netmask": "255.255.255.0", "dhcp": 0, "gw":"1.2.3.1" } }
      // extract actual JSON request data
      // - try POST data first
      JsonObjectPtr params = cmdObj->get("data");
      if (!params) {
        // no POST data, try uri_params
        params = cmdObj->get("uri_params");
      }
      // - extract command
      string cmd;
      if (checkStringParam(params, "cmd", cmd)) {
        // handle command
        err = handleJSONCmd(cmd, params, cmdObj, answer);
      }
      else {
        err = ErrorPtr(new Error(1,"Missing 'cmd'"));
      }
    }
    else {
      err = ErrorPtr(new Error(1,"Cannot decode JSON"));
    }
    if (!Error::isOK(err)) {
      // generate error message answer
      answer = makeErrorAnswer(err);
    }
    // return error or result answer if any
    if (answer) {
      answerAndTerminate(answer);
    }
  }


  JsonObjectPtr restart_from_ui(ErrorPtr &err, bool aPowerOff)
  {
    // try a soft reboot
    system_reboot(false, aPowerOff);
    // return confirmation that restart was initiated
    return emptyAnswer();
  }


  JsonObjectPtr factory_reset_from_ui(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    JsonObjectPtr o = aUriParams->get("mode");
    if (o){
      int mode = o->int32Value();
      if (mode>=1 && mode<=3) {
        // trigger running reset script
        factoryReset(mode);
        // return nothing, because app must not terminate until reset script has fully run
        return JsonObjectPtr();
      }
    }
    err = ErrorPtr(new Error(1,"Invalid or missing 'mode'"));
    return JsonObjectPtr();
  }


  #if BUILDENV_OPENWRT || BUILDENV_XCODE || BUILDENV_GENERIC

  // MARK: ===== time zone configuration

  JsonObjectPtr timezoneconfig(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    // check for parameters to set
    string tzcmd;
    JsonObjectPtr o;
    if (aUriParams->get("timezonename", o)) {
      // search for time zone spec
      string tzName = o->stringValue();
      const char *tzSpec = NULL;
      const TZInfo *tzP = timezones;
      while (tzP->tzname) {
        if (tzName==tzP->tzname) {
          tzSpec = tzP->tzspec;
          break;
        }
        tzP++;
      }
      if (!tzSpec) {
        err = ErrorPtr(new Error(1,"Unknown time zone name"));
      }
      else {
        // set
        #if BUILDENV_XCODE || BUILDENV_GENERIC
        answerAndTerminate(emptyAnswer());
        #else
        tzcmd = string_format(
          "uci set system.@system[0].zonename='%s';"
          "uci set system.@system[0].timezone='%s';"
          "uci commit system;"
          "echo $(uci -q get system.@system[0].timezone) >/tmp/TZ",
          tzName.c_str(),
          tzSpec
        );
        MainLoop::currentMainLoop().fork_and_system(
          boost::bind(&P44maintd::tzset_done, this, _1, _2),
          tzcmd.c_str(),
          true, NULL, // capture output to prevent output going to mg44
          0 // mute stderr
        );
        #endif
      }
    }
    else {
      // show current time zone
      #if BUILDENV_XCODE || BUILDENV_GENERIC
      // fake answer
      tzget_done(err, "Europe/Zurich");
      #else
      MainLoop::currentMainLoop().fork_and_system(
        boost::bind(&P44maintd::tzget_done, this, _1, _2),
        "uci -q get system.@system[0].zonename",
        true, NULL, // capture output to prevent output going to mg44
        0 // mute stderr
      );
      #endif
    }
    return JsonObjectPtr();
  }


  void tzget_done(ErrorPtr err, const string &aAnswer)
  {
    JsonObjectPtr result = JsonObject::newObj();
    result->add("timezonename", JsonObject::newString(trimWhiteSpace(aAnswer)));
    answerAndTerminate(makeAnswer(result));
  }



  void tzset_done(ErrorPtr err, const string &aAnswer)
  {
    // TZ successfully set, report success to Web UI
    answerAndTerminate(emptyAnswer());
  }


  #endif // BUILDENV_OPENWRT || BUILDENV_XCODE || BUILDENV_GENERIC



  // MARK: ===== network configuration


  bool addSetIpCmd(string &aSetIp, JsonObjectPtr aUriParams, const char *aBootVarName)
  {
    JsonObjectPtr o = aUriParams->get(aBootVarName);
    if (o) {
      string ipval = o->stringValue();
      // validate IP address
      struct sockaddr_in sa;
      int result = inet_pton(AF_INET, ipval.c_str(), &(sa.sin_addr));
      if (result==0) return false; // invalid IP
      // is valid
      #if BUILDENV_DIGIESP
      aSetIp += string_format("ubootenv --set '%s=%s';",aBootVarName, ipval.c_str());
      #elif BUILDENV_XCODE || BUILDENV_GENERIC
      aSetIp += string_format("echo set %s=%s; ",aBootVarName, ipval.c_str());
      #else
      // standard way is via p44ipconf
      aSetIp += string_format("p44ipconf %s %s;", aBootVarName, ipval.c_str());
      #endif
    }
    // no or valid IP
    return true;
  }


  JsonObjectPtr ipconfig(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    // check for parameters to set
    string setcmd;
    JsonObjectPtr o = aUriParams->get("dhcp");
    if (o) {
      // dhcp flag must be there or else we consider this only a query for current values
      bool ok = true;
      bool dhcp = o->boolValue();
      // first set DHCP flag
      #if BUILDENV_DIGIESP
      setcmd = string_format("ubootenv --set 'dhcp=%s';", dhcp ? "on" : "off");
      #elif BUILDENV_XCODE || BUILDENV_GENERIC
      setcmd = string_format("echo set dhcp=%s; ", dhcp ? "on" : "off");
      #else
      // standard way is via p44ipconf
      setcmd = string_format("p44ipconf dhcp %d;", dhcp ? 1 : 0);
      #endif
      // Set IP addresses
      if (!dhcp) {
        // manual IP
        ok = ok &&
          addSetIpCmd(setcmd, aUriParams, "ipaddr") &&
          addSetIpCmd(setcmd, aUriParams, "netmask") &&
          addSetIpCmd(setcmd, aUriParams, "gatewayip");
      }
      // always set DNS IPs
      ok = ok &&
        addSetIpCmd(setcmd, aUriParams, "dnsip") &&
        addSetIpCmd(setcmd, aUriParams, "dnsip2");
      // add ipv6
      bool ipv6 = false;
      if (aUriParams->get("ipv6", o)) {
        ipv6 = o->boolValue();
        setcmd += string_format("p44ipconf ipv6 %d;", ipv6);
      }
      #if BUILDENV_OPENWRT
      // need to commit
      setcmd += "p44ipconf commit now";
      #endif
      // now execute the set command
      LOG(LOG_DEBUG,"Executing IP config commands: %s", setcmd.c_str());
      if (ok) {
        MainLoop::currentMainLoop().fork_and_system(
          boost::bind(&P44maintd::cfgset_done, this, _1, _2),
          setcmd.c_str(),
          true, NULL, // capture output to prevent output going to mg44
          0 // mute stderr
        );
        return JsonObjectPtr(); // no answer now, but later when we get data
      }
      else {
        // not ok
        err = ErrorPtr(new Error(415, "Invalid IP address parameters"));
        return makeErrorAnswer(err); // error
      }
    }
    else {
      // query only
      MainLoop::currentMainLoop().fork_and_system(
        boost::bind(&P44maintd::ipquery_done, this, _1, _2),
        #if BUILDENV_DIGIESP
        "echo -n currentip=;ifconfig | sed -n -e 's/:127\\.0\\.0\\.1 //g' -e 's/ *inet addr:\\([0-9.]\\+\\).*/\\1/gp';/sbin/ubootenv --print 'dhcp ipaddr netmask gatewayip dnsip dnsip2'"
        #elif BUILDENV_XCODE
        "echo 'currentip=123.45.67.89'; echo 'dhcp=on'; echo 'ipv6=1'; echo 'ipaddr=192.168.42.99'; echo 'netmask=255.255.255.0'; echo 'gatewayip=192.168.42.1'; echo 'dnsip=8.8.8.8'; echo 'dnsip2=0.0.0.0'"
        #elif BUILDENV_GENERIC
        "echo 'currentip=123.42.42.42'; echo 'dhcp=on'; echo 'ipaddr=192.168.42.98'; echo 'netmask=255.255.255.0'; echo 'gatewayip=192.168.42.1'; echo 'dnsip=2.2.2.2'; echo 'dnsip2=0.0.0.0'"
        #else
        // standard way is via p44ipconf
        "p44ipconf"
        #endif
        , true, NULL, // capture output to prevent output going to mg44
        0 // mute stderr
      );
      return JsonObjectPtr(); // no answer now, but later when we get data
    }
  }



  static string getVar(const string &aStr, const string &aVarName)
  {
    string vn = aVarName + "=";
    size_t pos = aStr.find(vn);
    if (pos!=string::npos) {
      pos+=vn.size();
      size_t ep = aStr.find("\n",pos);
      if (ep==string::npos) {
        ep = aStr.size(); // to the end
      }
      return aStr.substr(pos,ep-pos);
    }
    else {
      return "";
    }
  }


  static string getIpVar(const string &aStr, const string &aVarName)
  {
    string v = getVar(aStr, aVarName);
    if (v.empty()) v="0.0.0.0";
    return v;
  }



  void ipquery_done(ErrorPtr aErr, const string &aAnswer)
  {
    // separate elements
    JsonObjectPtr result = JsonObject::newObj();
    result->add("currentip", JsonObject::newString(getDef("STATUS_IPV4")));
    result->add("dhcp", JsonObject::newBool(getVar(aAnswer, "dhcp")=="on"));
    result->add("ipv6", JsonObject::newBool(getVar(aAnswer, "ipv6")=="1"));
    result->add("ipaddr", JsonObject::newString(getIpVar(aAnswer, "ipaddr")));
    result->add("ipv6_link", JsonObject::newString(getVar(aAnswer, "ipv6_link")));
    result->add("ipv6_global", JsonObject::newString(getVar(aAnswer, "ipv6_global")));
    result->add("netmask", JsonObject::newString(getIpVar(aAnswer, "netmask")));
    result->add("gatewayip", JsonObject::newString(getIpVar(aAnswer, "gatewayip")));
    result->add("dnsip", JsonObject::newString(getIpVar(aAnswer, "dnsip")));
    result->add("dnsip2", JsonObject::newString(getIpVar(aAnswer, "dnsip2")));
    answerAndTerminate(makeAnswer(result));
  }


  void cfgset_done(ErrorPtr err, const string &aAnswer)
  {
    // IP parameters successfully set, report success to Web UI
    answerAndTerminate(emptyAnswer());
  }


  // MARK: ===== wifi settings

  #if !BUILDENV_DIGIESP

  JsonObjectPtr wificonfig(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    // check for parameters to set
    string setcmd;
    string iface = "cli";
    bool changes = false;
    for (int i = 0; i<2; i++) {
      JsonObjectPtr ifparams = aUriParams->get(iface.c_str());
      if (ifparams) {
        changes = true;
        JsonObjectPtr o;
        o = ifparams->get("enabled"); if (o) setcmd += string_format(" p44wificonf %s %d;", iface.c_str(), o->boolValue() ? 1 : 0);
        o = ifparams->get("ssid"); if (o) setcmd += string_format(" p44wificonf %s_ssid '%s';", iface.c_str(), o->stringValue().c_str());
        o = ifparams->get("encryption"); if (o) setcmd += string_format(" p44wificonf %s_encryption '%s';", iface.c_str(), o->stringValue().c_str());
        o = ifparams->get("key"); if (o) setcmd += string_format(" p44wificonf %s_key '%s';", iface.c_str(), o->stringValue().c_str());
      }
      iface = "ap";
    }
    if (changes) {
      // apply them
      setcmd += " p44wificonf commit now";
      #if BUILDENV_XCODE || BUILDENV_GENERIC
      setcmd = "echo "+shellQuote(setcmd)+" >/tmp/p44maint_p44wificonf";
      #endif
      // now execute the set command
      LOG(LOG_DEBUG,"Executing Wifi config commands: %s", setcmd.c_str());
      MainLoop::currentMainLoop().fork_and_system(
        boost::bind(&P44maintd::cfgset_done, this, _1, _2),
        setcmd.c_str(),
        true, NULL, // capture output to prevent output going to mg44
        0 // mute stderr
      );
      return JsonObjectPtr(); // no answer now, but later when we get data
    }
    else {
      // query only
      MainLoop::currentMainLoop().fork_and_system(
        boost::bind(&P44maintd::wifiquery_done, this, _1, _2),
        #if BUILDENV_XCODE || BUILDENV_GENERIC
        "echo 'cli=1'; echo 'cli_ssid=DUMMY'; echo 'cli_key=supersecret'; echo 'cli_encryption=psk2'; echo 'ap=0'; echo 'ap_ssid=AP_DUMMY'; echo 'ap_key='; echo 'ap_encryption=none';"
        #else
        // standard way is via p44ipconf
        "p44wificonf"
        #endif
        , true, NULL, // capture output to prevent output going to mg44
        0 // mute stderr
      );
      return JsonObjectPtr(); // no answer now, but later when we get data
    }
  }

  void wifiquery_done(ErrorPtr aErr, const string &aAnswer)
  {
    // separate elements
    JsonObjectPtr result = JsonObject::newObj();
    string iface = "cli";
    for (int i=0; i<2; i++) {
      JsonObjectPtr ifparams = JsonObject::newObj();
      ifparams->add("enabled", JsonObject::newBool(getVar(aAnswer, iface)=="1"));
      ifparams->add("ssid", JsonObject::newString(getVar(aAnswer, iface+"_ssid")));
      ifparams->add("encryption", JsonObject::newString(getVar(aAnswer, iface+"_encryption")));
      ifparams->add("key", JsonObject::newString(getVar(aAnswer, iface+"_key")));
      result->add(iface.c_str(), ifparams);
      iface = "ap";
    }
    answerAndTerminate(makeAnswer(result));
  }

  #endif // !BUILDENV_DIGIESP


  // MARK: ===== device information


  // show device info as text on console
  void showDeviceInfo()
  {
    printf("Model       : %s\n", getDef("PRODUCT_MODEL").c_str());
    printf("Variant     : %s\n", getDef("PRODUCT_VARIANT").c_str());
    printf("Producer    : %s\n", getDef("PRODUCER").c_str());
    printf("GTIN        : %s\n", getDef("PRODUCT_GTIN").c_str());
    printf("Serial      : %s\n", getDef("UNIT_SERIALNO").c_str());
    printf("Platform    : %s\n", getDef("PLATFORM_NAME").c_str());
    printf("OS          : %s\n", getDef("PLATFORM_OS_IDENTIFIER").c_str());
    printf("Firmware    : %s_%s\n", getDef("FIRMWARE_VERSION").c_str(), getDef("FIRMWARE_FEED").c_str());
    printf("hostname    : %s\n", getDef("UNIT_HOSTNAME").c_str());
    printf("IPv4        : %s\n", getDef("STATUS_IPV4").c_str());
    terminateApp(EXIT_SUCCESS);
  }


  void showDefs()
  {
    for (DefsMap::iterator pos = defs.begin(); pos!=defs.end(); pos++) {
      printf("%s=%s\n", pos->first.c_str(), shellQuote(pos->second).c_str());
    }
    terminateApp(EXIT_SUCCESS);
  }



  // return device info as JSON for web interface
  JsonObjectPtr devinfo(ErrorPtr &err)
  {
    JsonObjectPtr result = JsonObject::newObj();
    for (DefsMap::iterator pos = defs.begin(); pos!=defs.end(); pos++) {
      result->add(pos->first.c_str(), JsonObject::newString(pos->second));
    }
    // add time
    result->add("timetick", JsonObject::newInt64(time(NULL)));
    struct tm t;
    MainLoop::mainLoopTimeTolocalTime(MainLoop::now(), t);
    result->add("localtimetick", JsonObject::newInt64(time(NULL)+t.tm_gmtoff));
    // uptime
    int uptime = -1;
    #if BUILDENV_XCODE || BUILDENV_GENERIC
    uptime = 352800; // 4 days and 2 hours
    #else
    struct sysinfo info;
    sysinfo(&info);
    uptime = info.uptime;
    #endif
    result->add("uptime", JsonObject::newInt64(uptime));
    return makeAnswer(result);
  }



  // MARK: ===== password

  JsonObjectPtr setpassword(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    // check for parameters to set
    JsonObjectPtr o;
    string username;
    getDef("PRODUCT_WEBADMIN_USER", username,"vdcadmin");
    // optionally use different user name
    if (aUriParams->get("username", o)) {
      username = o->stringValue();
    }
    if (aUriParams->get("password", o)) {
      string password = o->stringValue();
      // use mg44 to modify global password file
      #if BUILDENV_XCODE || BUILDENV_GENERIC
      const char *path = "/bin/echo";
      const char *cmd[] = {
        "echo",
        "set user/password to",
        username.c_str(),
        "/",
        password.c_str(),
        NULL
      };
      #else
      // mg44 -A /flash/webui_authfile P44-xx-xx ${user} ${pw}
      string model = getDef("PRODUCT_MODEL");
      const char *path = "/usr/bin/mg44";
      const char *cmd[] = {
        "mg44",
        "-A", // create/edit auth file
        FLASH_PATH "webui_authfile", // auth file is on user file system
        model.c_str(), // model name as auth domain
        username.c_str(), // username
        password.c_str(), // password
        NULL
      };
      #endif
      MainLoop::currentMainLoop().fork_and_execve(
        boost::bind(&P44maintd::passwordUpdated, this, _1),
        path, (char **)cmd, NULL,
        true, NULL, // capture output to prevent output going to mg44
        0 // mute stderr
      );
      return JsonObjectPtr(); // no answer now, but later when we get data
    }
    err = ErrorPtr(new Error(1, "missing password"));
    return JsonObjectPtr();
  }


  void passwordUpdated(ErrorPtr aError)
  {
    if (Error::isOK(aError)) {
      answerAndTerminate(emptyAnswer());
    }
    else {
      answerAndTerminate(makeErrorAnswer(aError));
    }
  }


  // MARK: ===== persistent properties

  // generic key/value JSON property store
  JsonObjectPtr property(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    JsonObjectPtr o = aUriParams->get("key");
    if (!o) return emptyAnswer();
    string key = o->stringValue();
    if (key.find_first_of("/.")!=string::npos) return emptyAnswer(); // safeguard
    string file = FLASH_PATH "p44_property_" + lowerCase(key);
    if (aUriParams->get("value", o, false)) { // do not ignore NULL, we need it for delete
      if (o) {
        // set a new value
        string_tofile(file, o->json_str() + "\n");
      }
      else {
        // remove the value
        unlink(file.c_str());
      }
      return emptyAnswer();
    }
    else {
      // query the current value
      JsonObjectPtr v = JsonObject::objFromFile(file.c_str());
      if (v) return makeAnswer(v);
      return emptyAnswer();
    }
  }


  JsonObjectPtr getProperty(string aKey)
  {
    if (aKey.find_first_of("/.")!=string::npos) return JsonObjectPtr();
    string file = FLASH_PATH "p44_property_" + lowerCase(aKey);
    return JsonObject::objFromFile(file.c_str());
  }


  // MARK: ===== user level


  // query or set user level
  JsonObjectPtr userlevelaccess(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    JsonObjectPtr o = aUriParams->get("level");
    if (o) {
      // set the user level = write to /flash/p44userlevel
      int userlevel = o->int32Value();
      return statusAnswer(string_tofile(FLASH_PATH "p44userlevel", string_format("%d",userlevel)));
    }
    else {
      // query the level
      return makeAnswer(JsonObject::newString(string_format("%d",userlevel())));
    }
  }
  

  // MARK: ===== (pesistent) alerts

  #define ALERT_DIR "p44alerts/"

  JsonObjectPtr alert_from_ui(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    JsonObjectPtr o = aUriParams->get("new");
    if (o) {
      // create new alert, return ID
      return makeAnswer(JsonObject::newString(newAlert(o)));
    }
    o = aUriParams->get("confirm");
    if (o) {
      // confirm existing alert
      confirmAlert(o->stringValue());
      return emptyAnswer();
    }
    // return next pending alert
    return makeAnswer(nextAlert());
  }


  string newAlert(JsonObjectPtr aAlert)
  {
    string alertId;
    JsonObjectPtr o;
    if (aAlert->get("id", o)) {
      // id defined in the alert already -> use it
      alertId = o->stringValue();
    }
    else {
      // no predefined alert ID -> create random alert ID
      alertId = string_format("%lld_%d", MainLoop::now(), rand());
      aAlert->add("id", JsonObject::newString(alertId));
    }
    string alertFile = string_format(FLASH_PATH ALERT_DIR "alert_%s", alertId.c_str());
    mkdir(FLASH_PATH ALERT_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    string_tofile(alertFile, aAlert->json_str());
    return alertId;
  }

  bool confirmAlert(const string aAlertId)
  {
    if (aAlertId.find_first_of("/.")!=string::npos) return false; // safeguard
    string alertFile = string_format(FLASH_PATH ALERT_DIR "alert_%s", aAlertId.c_str());
    return (unlink(alertFile.c_str())==0);
  }


  JsonObjectPtr nextAlert()
  {
    string path = FLASH_PATH ALERT_DIR;
    DIR* alertDir = opendir(path.c_str());
    JsonObjectPtr alert;
    if (alertDir) {
      while (true) {
        struct dirent *ent = readdir(alertDir);
        if (!ent) break;
        if (ent->d_name[0]=='.') continue;
        // real file name
        pathstring_format_append(path, "%s", ent->d_name);
        alert = JsonObject::objFromFile(path.c_str());
        break;
      }
      closedir(alertDir);
    }
    return alert;
  }


  // MARK: ===== config backup & restore

  void config_backup(ErrorPtr &err)
  {
    // create filename
    string fn = getDef("UNIT_HOSTNAME") + "_" + string_ftime("%Y-%m-%d_%H.%M") + ".p44cfg";
    // create headers
    printf(
      "\x03" "application/octet-stream\r\n"
      "\x08" "Content-Disposition: attachment;filename=%s\r\n",
      fn.c_str()
    );
    fflush(stdout);
    // let backup script do the actual output directly
    // - call the update script now
    const char *path = "/bin/sh";
    const char *cmd[] = {
      "sh",
      "-c",
      #if BUILDENV_XCODE || BUILDENV_GENERIC
      "echo this is a dummy config file",
      #else
      "p44configbackup",
      #endif
      NULL
    };
    // exec the script
    // close all non-std file descriptors
    int fd = getdtablesize();
    while (fd>STDERR_FILENO) close(fd--);
    // change to the requested child process
    execve(path, (char **)cmd, environ); // replace process with new binary/script
    // execv returns only in case of error
    err = ErrorPtr(new Error(1,"Cannot exec backup script"));
  }


  JsonObjectPtr config_restore_prep(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    JsonObjectPtr o = aUriParams->get("uploadedfile");
    if (o) {
      string fn = o->stringValue();
      LOG(LOG_NOTICE, "calling config restore script (preparation phase)");
      string rcmd;
      #if BUILDENV_XCODE || BUILDENV_GENERIC
      rcmd = string_format("echo /tmp/config_restore"); // real restore script returns the prep dir path
//      rcmd = string_format("echo archive unusable; false"); // real restore script returns the prep dir path
      #else
      rcmd = string_format("p44configrestore --prepare \"%s\"", fn.c_str());
      #endif
      MainLoop::currentMainLoop().fork_and_system(boost::bind(&P44maintd::configPrepared, this, _1, _2), rcmd.c_str(), true);
      return JsonObjectPtr(); // no answer now, but later when we get data
    }
    err = ErrorPtr(new Error(1,"missing 'uploadedfile' param"));
    return JsonObjectPtr(); // no answer now, but later when we get data
  }


  long comparableVersion(const string aVersionStr)
  {
    long v = 0;
    const char *p = aVersionStr.c_str();
    string part;
    for (int i=0; i<4; i++) {
      if (!nextPart(p, part, '.')) break;
      int vp = atoi(part.c_str());
      switch (i) {
        case 0 : v += vp*10000000; break;
        case 1 : v += vp*100000; break;
        case 2 : v += vp*1000; break;
        case 3 : v += vp; break;
      }
    }
    return v;
  }


  void configPrepared(ErrorPtr aErr, const string &aResult)
  {
    if (Error::isOK(aErr)) {
      string prepdir = trimWhiteSpace(aResult);
      // checks
      bool oldArchive = false;
      bool differentModel = false;
      bool differentSerial = false;
      bool oldFirmware = false;
      // get defs from backup
      DefsMap cfgDefs;
      if (!readDefsFrom(prepdir + "/p44defs", cfgDefs)) {
        LOG(LOG_WARNING, "old config archive, does not have p44defs");
        oldArchive = true;
      }
      else {
        // we have read something from defs
        differentModel = !(getDef("PRODUCT_GTIN")==getDef("PRODUCT_GTIN", &cfgDefs));
        differentSerial = !(getDef("UNIT_SERIALNO")==getDef("UNIT_SERIALNO", &cfgDefs));
        oldFirmware = comparableVersion(getDef("FIRMWARE_VERSION")) < comparableVersion(getDef("FIRMWARE_VERSION", &cfgDefs));
      }
      // provide result
      JsonObjectPtr result = JsonObject::newObj();
      result->add("gtin", JsonObject::newString(getDef("PRODUCT_GTIN", &cfgDefs)));
      result->add("model", JsonObject::newString(getDef("PRODUCT_MODEL", &cfgDefs)));
      result->add("serial", JsonObject::newString(getDef("UNIT_SERIALNO", &cfgDefs)));
      result->add("version", JsonObject::newString(getDef("FIRMWARE_VERSION", &cfgDefs)));
      result->add("time", JsonObject::newString(getDef("STATUS_TIME", &cfgDefs)));
      result->add("oldarchive", JsonObject::newBool(oldArchive));
      result->add("differentmodel", JsonObject::newBool(differentModel));
      result->add("differentserial", JsonObject::newBool(differentSerial));
      result->add("oldfirmware", JsonObject::newBool(oldFirmware));
      answerAndTerminate(makeAnswer(result));
      return;
    }
    if (Error::isError(aErr, ExecError::domain(), 1)) {
      // use returned string as error message
      aErr = ErrorPtr(new Error(1, trimWhiteSpace(aResult)));
    }
    LOG(LOG_ERR, "config preparation failed: %s", aErr->description().c_str());
    answerAndTerminate(makeErrorAnswer(aErr));
  }



  JsonObjectPtr config_restore_apply(JsonObjectPtr aUriParams, ErrorPtr &err)
  {
    JsonObjectPtr o = aUriParams->get("mode");
    if (o){
      int mode = o->int32Value();
      if (mode>=0 && mode<=3) {
        #if BUILDENV_XCODE || BUILDENV_GENERIC
        printf("Real unit would execute: p44configrestore --apply %d", mode);
        answerAndTerminate(emptyAnswer());
        #else
        // exec the script to apply restore
        string c = string_format("p44configrestore --apply %d", mode);
        const char *path = "/bin/sh";
        const char *cmd[4] = {
          "sh",
          "-c",
          c.c_str(),
          NULL
        };
        // close all non-std file descriptors
        int fd = getdtablesize();
        while (fd>STDERR_FILENO) close(fd--);
        // change to the requested child process
        execve(path, (char **)cmd, environ); // replace process with new binary/script
        // execv returns only in case of error
        err = ErrorPtr(new Error(1,"Cannot exec restore apply script"));
        #endif
        return JsonObjectPtr();
      }
    }
    err = ErrorPtr(new Error(1,"missing or wrong 'mode'"));
    return JsonObjectPtr();
  }


  // MARK: ===== factory reset

  void factoryReset(int aMode)
  {
    // signal "busy"
    enableLEDs();
    redLED->steadyOn();
    greenLED->steadyOn();
    // do factory reset
    LOG(LOG_NOTICE, "calling factory reset script");
    string res;
    #if BUILDENV_XCODE || BUILDENV_GENERIC
    res = string_format("echo 'real platform would execute:' p44factoryreset %d", aMode);
    #else
    // call factory reset script
    res = string_format("p44factoryreset %d", aMode);
    #endif
    MainLoop::currentMainLoop().fork_and_system(
      boost::bind(&P44maintd::endApp, this, false), // exit with red LED on
      res.c_str()
    );
  }


  void endApp(bool aSuccess)
  {
    redLED->steadyOff();
    greenLED->steadyOff();
    // end with steady LED color according to exit status
    if (aSuccess) greenLED->steadyOn(); else redLED->steadyOn();
    // exit
    terminateApp(aSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
  }


}; // P44maintd
