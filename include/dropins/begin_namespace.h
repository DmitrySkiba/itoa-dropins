/*
 * Copyright (C) 2011 Dmitry Skiba
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Many IDEs have the bad habit of always indenting namespace
//  content. For global namespaces this is often undesirable.
//  One solution is to use following macros.

#ifndef BEGIN_NAMESPACE

#define BEGIN_NAMESPACE(name) \
    namespace name {
#define END_NAMESPACE(name) \
    }
    
#endif // BEGIN_NAMESPACE

