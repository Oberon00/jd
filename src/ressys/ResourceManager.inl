/// \file ResourceManager.inl Implementation file for ResourceManager.hpp

template<typename ResT>
ResourceManager<ResT>::ResourceManager():
    m_keepAll(false)
{
}

template<typename ResT>
ResourceManager<ResT>::~ResourceManager()
{
    if (!m_keepAll) {
        for (auto it = m_kept.cbegin(); it != m_kept.cend(); ++it)
            LOG_W("Resource \"" + it->first + "\" has not been released!");
    }
}

template<typename ResT>
typename ResourceManager<ResT>::Ptr ResourceManager<ResT>::request(
    std::string const& name)
{
    if (!m_callback)
        return get(name);

    WeakPtr& oldRes = m_resMap[name];
    if (oldRes.expired()) {
        LOG_D("Loading resource[" +
              std::string(typeid(ResT).name()) + "] \"" + name + "\"...");
        Ptr newRes(std::make_shared<ResT>());
        try {
            m_callback(*newRes, name);
            LOG_D("Finished loading resource \"" + name + "\".");
        } catch (std::exception const& ex) {
            LOG_EX(ex);
            LOG_E("Failed loading resource \"" + name + "\".");
            throw;
        }
        if (m_keepAll)
            m_kept.insert(std::make_pair(name, newRes));
        oldRes = newRes;
        return newRes;
    }
    else
        return oldRes.lock();
}

template<typename ResT>
void ResourceManager<ResT>::tidy()
{
     for (auto it = m_resMap.begin(); it != m_resMap.end();) {
         if (it->second.expired())
             it = m_resMap.erase(it);
         else ++it;
     }
}

template<typename ResT>
void ResourceManager<ResT>::purge()
{
    releaseAll();
    tidy();
}

template<typename ResT>
typename ResourceManager<ResT>::Ptr ResourceManager<ResT>::keepLoaded(
    std::string const& name)
{
    // insert and return inserted pointer
    Ptr result = request(name);
    m_kept.insert(std::make_pair(name, result));
    return result;
}

template<typename ResT>
void ResourceManager<ResT>::keepAllLoaded(bool const enable)
{
    m_keepAll = enable;
    if (enable)
        m_kept.insert(m_resMap.begin(), m_resMap.end());
}



template<typename ResT>
void ResourceManager<ResT>::release(std::string const& name)
{
    m_kept.erase(name);
}

template<typename ResT>
void ResourceManager<ResT>::releaseAll()
{
    m_kept.clear();
}

template<typename ResT>
typename ResourceManager<ResT>::Ptr ResourceManager<ResT>::tryGet(
    std::string const& name)
{
    auto const& p = m_resMap[name];
    if (p.expired())
        return nullptr;
    else
        return p.lock();
}


template<typename ResT>
typename ResourceManager<ResT>::Ptr ResourceManager<ResT>::get(
    std::string const& name)
{
    auto result = tryGet(name);
    if (!result)
        LOG_THROW(jd::ResourceError("resource \"" + name + "\" is not loaded"));
    else
        return result;
}

template<typename ResT>
void ResourceManager<ResT>::insert(
    std::string const& name, Ptr res)
{
    if (!res)
        LOG_THROW(jd::ResourceError("attempt to insert null pointer in ResourceManager"));
    m_resMap[name] = res;
    if (m_keepAll)
        m_kept[name] = res;
}

template<typename ResT>
typename ResourceManager<ResT>::ResourceNotFoundCallback
ResourceManager<ResT>::setResourceNotFoundCallback(
        ResourceNotFoundCallback const& callback)
{
    ResourceNotFoundCallback old(std::move(m_callback));
    m_callback = callback;
    return old;
}

template<typename ResT>
void ResourceManager<ResT>::useDefaultCallback()
{
    setResourceNotFoundCallback(&loadResource<ResT>);
}


//// FREE FUNCTIONS ////
template<typename ResT>
void loadResource(ResT& resource, std::string const& name)
{
    if (!resource.loadFromFile(name)) {
        throw jd::ResourceLoadError(
            "failed loading resource (type: " + std::string(typeid(ResT).name()) +
            ") from file \"" + name + "\".");
    }
}
