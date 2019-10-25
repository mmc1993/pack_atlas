#include "atlas_packer.h"

const std::array<AtlasPacker::Limit, 7> AtlasPacker::LimitConfig{
    Limit{256, 256, 1},
    Limit{512, 256, 1},
    Limit{512, 512, 3},
    Limit{1024, 512, 1},
    Limit{1024, 1024, 3},
    Limit{2048, 2048, 3},
    Limit{4096, 4096, 666},
};

std::vector<AtlasPacker::Atlas> AtlasPacker::Package(Items & items, uint offset, uint space, uint align)
{
    _align = align;
    _space = space;
    _offset = offset;

    Atlass result;
    for (auto level = 0u; !items.empty();)
    {
        Package(items, level, result);
        auto idx = CheckLimit(result);
        if (idx != result.size())
        {
            level = result.at(idx).mLevel + 1;
        }
        Breakup(items, result, idx);
    }
    return std::move(result);
}

void AtlasPacker::SortItems(Items & items)
{
    std::sort(items.begin(), items.end(), [](const Item & item0, const Item & item1)
        {
            return std::max(item0.w, item0.h) > std::max(item1.w, item1.h);
        });
}

void AtlasPacker::SortPacks(Packs & packs)
{
    std::sort(packs.begin(), packs.end(), [](const Pack & pack0, const Pack & pack1) 
        {
            return pack0.w * pack0.h > pack1.w * pack1.h;
        });
}

uint AtlasPacker::CheckLimit(Atlass & atlass)
{
    auto index = 0;
    auto count = 1u;
    auto level = atlass.front().mLevel;
    for (auto i = 0; i != atlass.size(); ++i, ++count)
    {
        if (atlass.at(i).mLevel != level)
        {
            index = i;
            count = 1u;
            level = atlass.at(i).mLevel;
        }
        if (count > LimitConfig[level].num)
        {
            return index;
        }
    }
    return atlass.size();
}

uint AtlasPacker::CheckLevel(uint level, const Item & item)
{
    if (_offset + item.w <= LimitConfig[level].w && _offset + item.h <= LimitConfig[level].h)
    {
        return level;
    }

    auto it = std::find_if(LimitConfig.begin(), LimitConfig.end(),
        [&item, this](const auto & limit)
        {
            return _offset + item.w <= limit.w && _offset + item.h <= limit.h;
        });
    return (uint)std::distance(LimitConfig.begin(), it);
}

uint AtlasPacker::CheckLevel(uint level, const Items & items)
{
    for (auto & item : items)
    {
        level = std::max(level, CheckLevel(level, item));
    }
    return level;
}

void AtlasPacker::Breakup(Items & items, Atlass & atlass, uint start)
{
    for (auto it = atlass.begin() + start; it != atlass.end(); ++it)
    {
        for (auto & pack : it->mPacks)
        {
            items.emplace_back(pack.v, pack.w, pack.h);
        }
    }
    atlass.erase(atlass.begin() + start, atlass.end());
}

void AtlasPacker::Package(Items & items, uint level, Atlass & atlass)
{
    SortItems(items);
    atlass.push_back(Package(items, level));
    while (!items.empty())
    {
        atlass.push_back(Package(items, 0));
    }
}

AtlasPacker::Atlas AtlasPacker::Package(Items & items, uint level)
{
    level = CheckLevel(level, items);
    Packs packs {
        {
            nullptr, 
            _offset, 
            _offset,
            LimitConfig[level].w - _offset,
            LimitConfig[level].h - _offset,
        },
    };

    Pack pack0;
    Pack pack1;
    Atlas atlas;
    atlas.mLevel = level;
    for (auto itemIt = items.begin(); itemIt != items.end();)
    {
        for (auto packIt = packs.begin(); packIt != packs.end();)
        {
            if (packIt->Package(*itemIt, &pack0, &pack1, _space, _align))
            {
                atlas.mPacks.push_back(*packIt);
                itemIt = items.erase(itemIt);
                packIt = packs.erase(packIt);
                packs.push_back(pack0);
                packs.push_back(pack1);
                goto next;
            }
            ++packIt;
        }
        ++itemIt;

        next:;
    }
    return std::move(atlas);
}
