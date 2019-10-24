#pragma once

#include "include.h"

//template <class T>
class AtlasPacker {
public:
    struct Item {
        void * v;
        uint w;
        uint h;
        Item(void * _v = nullptr, uint _w = 0, uint _h = 0)
            : v(_v), w(_w), h(_h) { }
    };

    struct Pack {
        void * v;
        uint x;
        uint y;
        uint w;
        uint h;

        Pack(void * _v = nullptr,
             uint _x = 0, uint _y = 0, 
             uint _w = 0, uint _h = 0)
            : x(_x), y(_y), w(_w), h(_h), v(_v)
        { }

        uint ApplyAlign(uint v, uint align)
        {
            return (uint)std::ceil(v / (float)align) * align;
        }

        bool IsContains(const Item & item, uint align)
        {
            auto iw = ApplyAlign(item.w, align);
            auto ih = ApplyAlign(item.h, align);
            return w >= iw && h >= ih;
        }

        bool Package(const Item & item, Pack * out0, Pack * out1, uint space, uint align)
        {
            if (!IsContains(item, align))
            {
                return false;
            }
            out0->x = out0->y = out0->w = out0->h = 0;
            out1->x = out1->y = out1->w = out1->h = 0;

            auto iw = ApplyAlign(item.w, align);
            auto ih = ApplyAlign(item.h, align);
            auto cx = x + iw + space;
            auto cy = y + ih + space;
            auto s0 = (w - iw - space) * ih;
            auto s1 = (h - ih - space) * w;
            auto s2 = (w - iw - space) * h;
            auto s3 = (h - ih - space) * iw;
            if (std::max(s0, s1) > std::max(s2, s3))
            {
                if (w - iw >= space)
                {
                    out0->x = cx;
                    out0->y = y;
                    out0->w = w - iw - space;
                    out0->h = ih;
                }
                if (h - ih >= space)
                {
                    out1->x = x;
                    out1->y = cy;
                    out1->w = w;
                    out1->h = h - ih - space;
                }
            }
            else
            {
                if (w - iw >= space)
                {
                    out0->x = cx;
                    out0->y = y;
                    out0->w = w - iw - space;
                    out0->h = h;
                }
                if (h - ih >= space)
                {
                    out1->x = x;
                    out1->y = cy;
                    out1->w = iw;
                    out1->h = h - ih - space;
                }
            }
            w = iw; h = ih; v = item.v;
            return true;
        }
    };

    struct Atlas { 
        uint              mLevel; 
        std::vector<Pack> mPacks; 
    };

    using Items = std::vector<Item>;
    using Packs = std::vector<Pack>;
    using Atlass = std::vector<Atlas>;

    //  打包级别
    struct Limit {
        uint w;
        uint h;
        uint num;
        Limit(uint _w = 0, uint _h = 0, uint _num = 0)
            : w(_w), h(_h), num(_num) { }
    };

    static const std::array<Limit, 8> LimitConfig;

    Atlass Package(Items & items, uint offset, uint space, uint align);

private:
    void SortItems(Items & items);
    void SortPacks(Packs & packs);
    uint CheckLimit(Atlass & atlass);
    uint CheckLevel(uint level, const Item  & item);
    uint CheckLevel(uint level, const Items & items);
    void  Breakup(Items & items, Atlass & atlass, uint start);
    void  Package(Items & items, uint level, Atlass & atlass);
    Atlas Package(Items & items, uint level);

private:
    uint _align;
    uint _space;
    uint _offset;
};
