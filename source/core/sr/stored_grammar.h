//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//

#pragma once
#include <string>
#include "ispxinterfaces.h"
#include "interface_helpers.h"

namespace Microsoft {
namespace CognitiveServices {
namespace Speech {
namespace Impl {

class CSpxStoredGrammar : public ISpxStoredGrammar, public ISpxGrammar
{
public:

    CSpxStoredGrammar() = default;

    SPX_INTERFACE_MAP_BEGIN()
        SPX_INTERFACE_MAP_ENTRY(ISpxStoredGrammar)
        SPX_INTERFACE_MAP_ENTRY(ISpxGrammar)
    SPX_INTERFACE_MAP_END()

    void InitStoredGrammar(const wchar_t *id) override;
    std::list<std::string> GetListenForList() override;

private:

    DISABLE_COPY_AND_MOVE(CSpxStoredGrammar);

    std::string m_id;
};

}}}} // Microsoft::CognitiveServices::Speech::Impl
#pragma once