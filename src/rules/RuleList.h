
#include <QList>

#include "RuleRepository.h"
#include "RuleMatch.h"
#include "Rules.h"

class QString;

class RuleList
{
    
public:
    
  RuleList( const QString& filenames);
  ~RuleList();

  const QList<RuleRepository> getAllRepositories() const;
  const QList<QList<RuleMatch> > getAllMatchRules() const;
  const QList<Rules*> getRules() const;
  void load();

private:
    
  QString filenames;
  QList<Rules*> rules;
  QList<RuleRepository> allrepositories;
  QList<QList<RuleMatch> > allMatchRules;
  
};
