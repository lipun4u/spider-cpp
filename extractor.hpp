#ifndef SPIDER_EXTRACTOR_HPP
#define SPIDER_EXTRACTOR_HPP

#include <algorithm>
#include <string>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include "url.hpp"

namespace spider {

    class UrlExtractor {
    public:
        UrlExtractor();
        virtual ~UrlExtractor();

        virtual void getUrls(
            Url const& baseAddress,
            std::string const& content,
            std::vector<Url> & destination) const = 0;

    };

    class TagUrlExtractor : public virtual UrlExtractor{
        boost::regex m_regex;

        Url buildUrl(Url const& baseAddress, boost::smatch const& match) const;

    public:
        explicit TagUrlExtractor(
            std::string const& tagName,
            std::string const& attributeName);

        virtual void getUrls(
            Url const& baseAddress,
            std::string const& content,
            std::vector<Url> & destination) const;
    };

    class CompoundExtractor : public virtual UrlExtractor {
        std::vector<boost::shared_ptr<UrlExtractor> > m_extractors;

    public:
        void addExtractor(boost::shared_ptr<UrlExtractor> const& extractor);

        virtual void getUrls(
            Url const& baseAddress,
            std::string const& content,
            std::vector<Url> & destination) const;
    };

}

#endif // SPIDER_EXTRACTOR_HPP
